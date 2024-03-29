#ifndef AVM_VIDEOCAP_H
#define AVM_VIDEOCAP_H

#include "common.h"
#include "System.h"


class System;

class VideoCap
{
public:

    cv::cuda::GpuMat mMap1, mMap2;  // for undistort
    cv::cuda::GpuMat mMapx, mMapy;  // for warpPerspective  
    // cv::cuda::GpuMat mProjectMat;
    cv::Mat mProjectMat;
    cv::Size mResolution;   

    VideoCap();
    VideoCap(const char *c, const std::string &n, const std::string &cp, const std::string &sp);
    ~VideoCap();

    cv::cuda::GpuMat* GetImage();
    void SetStop();
    void Run();
    void GetUndistortMap(cv::Mat camera_matrix_,
                         cv::Mat dist_coeffs_,
                         cv::Mat scale_xy_,
                         cv::Mat shift_xy_);
    void ParseConfig();
    
private:
    friend class System;

    mutable std::mutex mmBgr_;
    mutable std::mutex mmPro_;
    mutable std::mutex iomutex;

    const char * mId_;
    std::string mName_;
    std::string mConfigPath_;
    std::string mSavePathRaw_;

    bool mbStop_;

    std::deque<cv::Mat*> *mBgrs_;
    std::deque<cv::cuda::GpuMat*> *mPros_;

};


#endif