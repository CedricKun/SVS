#ifndef AVM_VIDEOCAP_H
#define AVM_VIDEOCAP_H

#include "common.h"
#include "System.h"


class System;

class VideoCap
{
public:

    cv::cuda::GpuMat mFrame;
    cv::cuda::GpuMat mMap1, mMap2;  // for undistort
    cv::Mat mProjectMat;
    cv::Size mResolution;   
    cv::Size mShape;

    VideoCap();
    VideoCap(const char *c, const std::string &n, const std::string &cp, const std::string &sp);
    ~VideoCap();

    void GetImage(cv::cuda::GpuMat &f);
    void SetStop();
    void Run();
    void GetUndistortMap(cv::Mat camera_matrix_,
                         cv::Mat dist_coeffs_,
                         cv::Mat scale_xy_,
                         cv::Mat shift_xy_);
    void ParseConfig();
    
private:
    friend class System;

    mutable std::mutex iomutex;
    std::condition_variable cv;

    const char * mId_;
    std::string mName_;
    std::string mConfigPath_;
    std::string mSavePathRaw_;

    bool mbStop_;
    bool mbRead_;
};


#endif