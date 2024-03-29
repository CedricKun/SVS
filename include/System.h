#ifndef AVM_SYSTEM_H
#define AVM_SYSTEM_H

#include "common.h"
#include "VideoCap.h"

class VideoCap;

class System
{
public:
    System(const char *argv[], size_t num);
    void LoadWeights();
    void StitchImg();
    void StitchImg(const std::vector<cv::Mat> &photos);
    void Start();
    std::vector<VideoCap*> mCaps;

    ~System();
private:
    mutable std::mutex mStitch;
    std::string mConfigPath;
    std::string mSavePath;

    std::vector<cv::cuda::GpuMat> mWeights;
    cv::Mat mAVM;

    size_t mNumViews;


};



#endif