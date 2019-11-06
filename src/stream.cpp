//
// Created by gh on 10/28/19.
//
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


#include "stream.h"

namespace stream{

    unsigned long get_tick_count(){
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    }

    int interrupt(void *ctx){
        if (ctx == nullptr) {
            return AVERROR_EOF;
        }

        auto now =  get_tick_count();
        stream::Base* pStream = (stream::Base*)ctx;
        if (now - pStream->GetLastTickCount() > 30 * 1000) {
            std::cout << "av_frame interrupt"<<std::endl;
            return AVERROR_EOF;
        }
        return 0;
    }

    int Base::openCam(int id) {
        return -1;
    }

    int Base::openFile(std::string file) {
        return -1;
    }

    int Base::openRtsp(std::string url) {
        return -1;
    }


    unsigned long Base::GetLastTickCount(){
        return lastTickCount_;
    }

//    Base::~Base() {
//
//    }

}