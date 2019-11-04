cmake_minimum_required(VERSION 2.8)

project("hwdemo")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O2 -fpermissive -std=c++11")

find_package(PkgConfig)


pkg_search_module(AVCODEC REQUIRED libavcodec)
include_directories(${AVCODEC_INCLUDE_DIRS})
pkg_search_module(AVFORMAT REQUIRED libavformat)
include_directories(${AVFORMAT_INCLUDE_DIRS})
link_directories(${AVFORMAT_LIBRARY_DIRS})

pkg_search_module(AVDEVICE REQUIRED libavdevice)
include_directories(${AVDEVICE_INCLUDE_DIRS})

pkg_search_module(AVFILTER REQUIRED libavfilter)
include_directories(${AVFILTER_INCLUDE_DIRS})

pkg_search_module(AVUTIL REQUIRED libavutil)
include_directories(${AVUTIL_INCLUDE_DIRS})

pkg_search_module(SWRESAMPLE REQUIRED libswresample)
include_directories(${SWRESAMPLE_INCLUDE_DIRS})

pkg_search_module(SWSCALE REQUIRED libswscale)
include_directories(${SWSCALE_INCLUDE_DIRS})

pkg_search_module(OPENCV REQUIRED opencv)
include_directories(${OPENCV_INCLUDE_DIRS})
link_directories(${OPENCV_LIBRARY_DIRS})

include_directories(include)


add_executable(TEST main.cpp include/stream.h src/stream.cpp include/NVStream.h src/NVStream.cpp)
target_link_libraries(TEST    ${OPENCV_LIBRARIES} ${AVCODEC_LIBRARIES}  ${AVFORMAT_LIBRARIES} ${AVDEVICE_LIBRARIES} ${AVFILTER_LIBRARIES} ${AVUTIL_LIBRARIES} ${SWRESAMPLE_LIBRARIES} ${SWSCALE_LIBRARIES} )
