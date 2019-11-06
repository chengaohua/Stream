//
// Created by gh on 10/28/19.
//

#ifndef HWDEMO_STREAM_H
#define HWDEMO_STREAM_H

#include <string>
#include <opencv2/opencv.hpp>

namespace stream {

    unsigned long get_tick_count();
    int interrupt(void *ctx);


    class Base {
    public:

        virtual  int openCam(int id);
        virtual  int openFile(std::string file);
        virtual  int openRtsp(std::string );

        virtual int getFrame(cv::Mat & frame) = 0;

        virtual int closeStream() = 0;

        virtual ~Base() = default;

        unsigned long GetLastTickCount() ;


    protected:
        unsigned long       lastTickCount_;

    };

}


#endif //HWDEMO_STREAM_H
