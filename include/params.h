#ifndef AVM_PARAMS_H
#define AVM_PARAMS_H

#include <vector>
#include <string>
#include <map>
#include <opencv2/opencv.hpp>


//单个格子10cm
// 后侧标定布边缘->车尾175cm
// 左9.5cm 右13cm 前170cm
//--------------------------------------------------------------------
//(shift_width, shift_height): how far away the birdview looks outside
//of the calibration pattern in horizontal and vertical directions
static const  int shift_w = 495;
static const  int shift_h = 325;

static const  int cali_map_w  = 510;    // 610
static const  int cali_map_h  = 850;   // 830
//size of the gap between the calibration pattern and the car
//in horizontal and vertical directions
static const  int inn_shift_w = 7;
static const  int inn_shift_h = 49;

//total width/height of the stitched image
static const  int total_w = cali_map_w + 2 * shift_w;
static const  int total_h = cali_map_h + 2 * shift_h;

//four corners of the rectangular region occupied by the car
//top-left (x_left, y_top), bottom-right (x_right, y_bottom)
static const  int xl = shift_w + 150 + inn_shift_w;
static const  int xr = total_w - xl;
static const  int yt = shift_h + 150 + inn_shift_h;
static const  int yb = total_h - yt;


// static const std::vector<const char *> vConfigs = {
//     "/dev/video0",
//     "/dev/video2",
//     "/dev/video6",
//     "/dev/video7",
// };

static const std::vector<const char *> vConfigs = {
    "nvv4l2camerasrc device=/dev/video2 ! video/x-raw(memory:NVMM),format=YUY2, width=1920,height=1080,framerate=30/1 ! nvvidconv ! video/x-raw,format=BGRx ! videoconvert ! video/x-raw, format=BGR ! appsink drop=1",
    "nvv4l2camerasrc device=/dev/video3 ! video/x-raw(memory:NVMM),format=YUY2, width=1920,height=1080,framerate=30/1 ! nvvidconv ! video/x-raw,format=BGRx ! videoconvert ! video/x-raw, format=BGR ! appsink drop=1",
    "nvv4l2camerasrc device=/dev/video0 ! video/x-raw(memory:NVMM),format=YUY2, width=1920,height=1080,framerate=30/1 ! nvvidconv ! video/x-raw,format=BGRx ! videoconvert ! video/x-raw, format=BGR ! appsink drop=1",
    "nvv4l2camerasrc device=/dev/video1 ! video/x-raw(memory:NVMM),format=YUY2, width=1920,height=1080,framerate=30/1 ! nvvidconv ! video/x-raw,format=BGRx ! videoconvert ! video/x-raw, format=BGR ! appsink drop=1",
};

static const std::vector<std::string> vNames = {
    "front", "back", "left", "right"
};



static const std::map<std::string, cv::Size> project_shapes = {
    {"front",  cv::Size(total_w, yt)},
    {"back",   cv::Size(total_w, yt)},
    {"left",   cv::Size(total_h, xl)},
    {"right",  cv::Size(total_h, xl)},
};

static const std::map<std::string, std::vector<cv::Point2f>> project_keypoints = {
    {"front", {cv::Point2f(shift_w + 195, shift_h + 25),
              cv::Point2f(shift_w + 315, shift_h + 125),
              cv::Point2f(shift_w + 195, shift_h + 25),
              cv::Point2f(shift_w + 315, shift_h + 125)}},

    {"back", {cv::Point2f(shift_w + 195, shift_h + 25),
              cv::Point2f(shift_w + 315, shift_h + 125),
              cv::Point2f(shift_w + 195, shift_h + 25),
              cv::Point2f(shift_w + 315, shift_h + 125)}},

    {"left", {cv::Point2f(shift_w + 465, shift_h + 25),
              cv::Point2f(shift_w + 625, shift_h + 125),
              cv::Point2f(shift_w + 465, shift_h + 25),
              cv::Point2f(shift_w + 625, shift_h + 125)}},

    {"right", {cv::Point2f(shift_w + 235, shift_h + 25),
              cv::Point2f(shift_w + 395, shift_h + 125),
              cv::Point2f(shift_w + 235, shift_h + 25),
              cv::Point2f(shift_w + 395, shift_h + 125)}}
};

#endif