//
// Created by gh on 10/28/19.
//

#ifndef HWDEMO_NVSTREAM_H
#define HWDEMO_NVSTREAM_H

#include "stream.h"

namespace stream{

    class NVStream: public Base {

        virtual int openCam(int id) override;
        virtual  int openFile(std::string file) override;
        virtual  int openRtsp(std::string ) override;

        virtual int getFrame(cv::Mat & frame) override;

        virtual int closeStream() override;

    };
}

#endif //HWDEMO_NVSTREAM_H
