#include "NVStream.h"

int main(int argc, char ** argv) {

    stream::NVStream *stream = new stream::NVStream;
    stream->openRtsp("rtmp://192.168.6.13/live/livestream");
    while (true) {
        cv::Mat frame;
        stream->getFrame(frame);

//        if(!frame.empty()) {
//            cv::imshow("frame", frame);
//            cv::waitKey(1);
//        }

    }


    return 0;
}