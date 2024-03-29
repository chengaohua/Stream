//
// Created by gh on 11/4/19.
//

extern "C" {
#include <libavutil/imgutils.h>
}

#include "NVStream.h"

//static enum AVPixelFormat hw_pix_fmt;

//static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
//                                        const enum AVPixelFormat *pix_fmts)
//{
//    const enum AVPixelFormat *p;
//
//    for (p = pix_fmts; *p != -1; p++) {
//        if (*p == hw_pix_fmt)
//            return *p;
//    }
//
//    fprintf(stderr, "Failed to get HW surface format.\n");
//    return AV_PIX_FMT_NONE;
//}

namespace stream {
    int NVStream::openRtsp(std::string url) {
        if (url.size() <= 7) {
            std::cout << "CStream::open err url:" << url << std::endl;
            return -1;
        }

        input_ctx_ = avformat_alloc_context();
        input_ctx_->interrupt_callback.callback = interrupt;
        input_ctx_->interrupt_callback.opaque = this;
        lastTickCount_ = get_tick_count();

        AVDictionary *options = nullptr;
        av_dict_set(&options, "buffer_size", "10485760", 0);

        std::cout << "CStream::open rtsp url:" << url << std::endl;
        av_dict_set(&options, "rtsp_transport", "tcp", 0);
        av_dict_set(&options, "stimeout", "2000000", 0);


        //AVInputFormat *avfmt = nullptr;
        if (int errCode = avformat_open_input(&input_ctx_, url.c_str(), nullptr, &options) != 0) {

            av_dict_free(&options);
            avformat_free_context(input_ctx_);
            return false;
        }

        av_dict_free(&options);
        std::cout << "CStream::open avformat_open_input success url:" << url << std::endl;

        if (0 < avformat_find_stream_info(input_ctx_, nullptr)) {
            std::cout << "CStream::open avformat_open_input failed:" << url << std::endl;
            avformat_free_context(input_ctx_);
            return false;
        }
        std::cout << "CStream::open avformat_find_stream_info success url:" << url << std::endl;


        /* find the video stream information */
        auto ret = av_find_best_stream(input_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder_, 0);
        if (ret < 0) {
            fprintf(stderr, "Cannot find a video stream in the input file\n");
            return -1;
        }
        video_stream_ = ret;


        AVHWDeviceType type = AV_HWDEVICE_TYPE_CUDA;
        for (int i = 0;; i++) {
            const AVCodecHWConfig *config = avcodec_get_hw_config(decoder_, i);
            if (!config) {
                fprintf(stderr, "Decoder %s does not support device type %s.\n",
                        decoder_->name, av_hwdevice_get_type_name(type));
                return -1;
            }
            if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                config->device_type == type) {

                hw_pix_fmt_ = config->pix_fmt;
                break;
            }
        }

        if (!(decoder_ctx_ = avcodec_alloc_context3(decoder_)))
            return AVERROR(ENOMEM);

        av_stream_ = input_ctx_->streams[video_stream_];
        if (avcodec_parameters_to_context(decoder_ctx_, av_stream_->codecpar) < 0)
            return -1;

       // hw_pix_fmt = hw_pix_fmt_;
        //decoder_ctx_->get_format = get_hw_format;
        decoder_ctx_->pix_fmt = hw_pix_fmt_;

        if ((ret = av_hwdevice_ctx_create(&hw_device_ctx_, type,
                                          NULL, NULL, 0)) < 0) {
            fprintf(stderr, "Failed to create specified HW device.\n");
            return ret;
        }
        decoder_ctx_->hw_device_ctx = av_buffer_ref(hw_device_ctx_);

        if ((ret = avcodec_open2(decoder_ctx_, decoder_, NULL)) < 0) {
            fprintf(stderr, "Failed to open codec for stream #%u\n", video_stream_);
            return -1;
        }


        std::cout<<"open rtsp success!"<<std::endl;
        return 0;
    }


    int NVStream::getFrame(cv::Mat &frame) {

        double start = cv::getTickCount();
        int ret;
        if (( ret = av_read_frame(input_ctx_, &packet_)) < 0)
            return -1;

        if (video_stream_ == packet_.stream_index)
            ret = decode_packet(decoder_ctx_, &packet_);
        av_packet_unref(&packet_);

        return ret;


    }


     int NVStream::decode_packet(AVCodecContext *avctx, AVPacket *packet)
    {
        AVFrame *frame = NULL, *sw_frame = NULL;
        AVFrame *tmp_frame = NULL;
        uint8_t *buffer = NULL;

        uint8_t *dst_data[4];
        dst_data[0] = nullptr;
        dst_data[1] = nullptr;
        dst_data[2] = nullptr;
        dst_data[3] = nullptr;
        int dst_linesize[4];

        int size;
        int ret = 0;

        do {

            ret = avcodec_send_packet(avctx, packet);
            if (ret < 0) {
                fprintf(stderr, "Error during decoding\n");
                break;
            }

            if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc())) {
                fprintf(stderr, "Can not alloc frame\n");
                ret = AVERROR(ENOMEM);
                break;
            }

            ret = avcodec_receive_frame(avctx, frame);
            if (ret < 0) {
                fprintf(stderr, "Error while decoding\n");
               break;
            }

            if (frame->format == hw_pix_fmt_) {
                //  sw_frame->format = AV_PIX_FMT_BGR24;
                /* retrieve data from GPU to CPU */
                if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
                    fprintf(stderr, "Error transferring the data to system memory\n");
                    break;
                }
                tmp_frame = sw_frame;
            } else
                tmp_frame = frame;


            size = av_image_get_buffer_size(tmp_frame->format, tmp_frame->width,
                                            tmp_frame->height, tmp_frame->linesize);

            buffer = av_malloc(size);
            if (!buffer) {
                fprintf(stderr, "Can not alloc buffer\n");
                ret = AVERROR(ENOMEM);
                break;
            }
            ret = av_image_copy_to_buffer(buffer, size,
                                          (const uint8_t * const *)tmp_frame->data,
                                          (const int *)tmp_frame->linesize, tmp_frame->format,
                                          tmp_frame->width, tmp_frame->height, tmp_frame->linesize);
            if (ret < 0) {
                fprintf(stderr, "Can not copy image to buffer\n");
               break;
            }



            if (av_image_alloc(dst_data, dst_linesize, tmp_frame->width,
                               tmp_frame->height, AV_PIX_FMT_BGR24, 1) < 0) {
                ret = AVERROR(ENOMEM);
                break;
            }




          //  double start = cv::getTickCount();
            m_swsContext_ = sws_getCachedContext(m_swsContext_,
                                                tmp_frame->width, tmp_frame->height,
                                                tmp_frame->format,
                                                tmp_frame->width, tmp_frame->height,
                                                AV_PIX_FMT_BGR24, SWS_POINT,
                                                NULL, NULL, NULL);

            sws_scale(m_swsContext_, tmp_frame->data, tmp_frame->linesize,
                      0, tmp_frame->height, dst_data, dst_linesize);

         //   double end = cv::getTickCount();
          //  std::cout<<"sws_scale run time = "<<(end-start)/cv::getTickFrequency() * 1000<<" ms"<<std::endl;

            cv::Mat tmp(tmp_frame->height, tmp_frame->width, CV_8UC3, (char *) dst_data[0]);

            cv::imshow("test",tmp);
            cv::waitKey(1);


        }while (0);

        if(frame) {
            av_frame_free(&frame);
        }

        if(sw_frame) {
            av_frame_free(&sw_frame);
        }

        if(buffer) {
            av_freep(&buffer);
        }

        if(dst_data[0]) {
            av_freep(&dst_data[0]);
        }


        lastTickCount_ = get_tick_count();

        return ret;
    }




    int NVStream::closeStream() {
        return 0;
    }

    int NVStream::openFile(std::string file) {
        return 0;
    }

    int NVStream::openCam(int id) {
        return 0;
    }

}
