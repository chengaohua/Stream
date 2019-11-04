//
// Created by gh on 10/28/19.
//

#ifndef HWDEMO_NVSTREAM_H
#define HWDEMO_NVSTREAM_H

#include "stream.h"

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
}


namespace stream{

    class NVStream: public Base {

    public:
        virtual int openCam(int id) override;
        virtual  int openFile(std::string file) override;
        virtual  int openRtsp(std::string ) override;

        virtual int getFrame(cv::Mat & frame) override;

        virtual int closeStream() override;

    private:

        AVFormatContext * input_ctx_;
        AVStream * av_stream_;
        int video_stream_;
        AVCodecContext * decoder_ctx_;
        AVCodec * decoder_;
        AVPacket packet_;
        enum AVPixelFormat hw_pix_fmt_;
        AVBufferRef *hw_device_ctx_ ;
        SwsContext*         m_swsContext_;


    };
}

#endif //HWDEMO_NVSTREAM_H
