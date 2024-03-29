#include "VideoCap.h"

#include "common.h"
#include "params.h"


VideoCap::VideoCap(){};

VideoCap::~VideoCap(){};

VideoCap::VideoCap(const char *c,
                   const std::string &n, 
                   const std::string &cp, 
                   const std::string &sp):
                   mId_(c), mName_(n), mConfigPath_(cp), 
                   mSavePathRaw_(sp), mbStop_(false){
        mConfigPath_ = mConfigPath_ + mName_ + ".yaml";
        mSavePathRaw_ = mSavePathRaw_ + mName_ + "/";

        ParseConfig();
        mBgrs_ = new std::deque<cv::Mat*>();
        mPros_ = new std::deque<cv::cuda::GpuMat*>();
    }

void VideoCap::ParseConfig(){

    if(access(mSavePathRaw_.data(), 0)){
        char path[300] = "mkdir -p ";
        std::cout << "raw data save path is not existed\n";
        int ret = system(strcat(path, mSavePathRaw_.c_str()));
    }

    cv::FileStorage fConfig(mConfigPath_, cv::FileStorage::READ);
    if(!fConfig.isOpened()){
      std::cerr << "ERROR: Wrong path at : "  << std::endl;
      exit(-1);
    }
    
    cv::Mat camera_matrix_ = fConfig["camera_matrix"].mat();
    cv::Mat dist_coeffs_ = fConfig["dist_coeffs"].mat();
    cv::Mat project_matrix_ = fConfig["project_matrix"].mat();
    auto r = fConfig["resolution"].mat();
    mResolution = cv::Size(r.at<int>(0), r.at<int>(1));
    cv::Mat scale_xy_ = fConfig["scale_xy"].mat();
    cv::Mat shift_xy_ = fConfig["shift_xy"].mat();
    GetUndistortMap(camera_matrix_,
                    dist_coeffs_,
                    scale_xy_,
                    shift_xy_);
    mProjectMat = project_matrix_;
    cv::cuda::buildWarpPerspectiveMaps(project_matrix_, false, mResolution, mMapx, mMapy);
    fConfig.release();
    // cout << "Finish Parsing Config\n";
}

void VideoCap::Run()
{
    // std::cout << "Thread #: on CPU " << sched_getcpu() << "\n";
	// create input stream
    cv::VideoCapture cap(mId_, cv::CAP_GSTREAMER);
    if (!cap.isOpened())  
    {  
        std::cout << "Failed to open camera.\n";  
        return;  
    }  

    // timestamps saving
    std::ofstream timeFile;
    std::string time_log = mSavePathRaw_ + "timestamp.txt";
    timeFile.open(time_log.data());

    int frame_id = 0;
    char ad[300];

    // printf("%d  %d", mResolution.height, mResolution.width);
    cv::cuda::GpuMat frame_in(mResolution.height, mResolution.width, CV_32FC3);
    cv::cuda::GpuMat frame_out(mResolution.height, mResolution.width, CV_32FC3);
	/*
	 * processing loop
	 */
    while(cap.isOpened() && !mbStop_)
    {
        cv::cuda::GpuMat *frame_pro = new cv::cuda::GpuMat(mResolution.height, mResolution.width, CV_32FC3);
        // std::cout << frame_pro->size() << std::endl;
        cv::Mat *frame = new cv::Mat(); 

		// capture next image
        cap >> *frame; 

		// log timing
        std::string t = ComputeTime();
        std::cout <<  mName_ << " frame id " << frame_id++ << " captured at: " << t << std::endl;

        // pass &frame to dequeue


        // processing frames on G
        frame_in.upload(*frame);
        cv::cuda::remap(frame_in, frame_out, mMap1, mMap2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);
        cv::cuda::warpPerspective(frame_out, *frame_pro, mProjectMat, project_shapes.at(mName_));
        if(mName_ == "left"){
            cv::cuda::rotate(*frame_pro, *frame_pro, cv::Size(project_shapes.at(mName_).height, project_shapes.at(mName_).width),
                             90, 0, project_shapes.at(mName_).width, cv::INTER_LINEAR);
        }
        else if(mName_ == "right"){
            cv::cuda::rotate(*frame_pro, *frame_pro, cv::Size(project_shapes.at(mName_).height, project_shapes.at(mName_).width),
                             -90, project_shapes.at(mName_).height - 1, 0, cv::INTER_LINEAR);
        }
        else if(mName_ == "back"){
            cv::cuda::rotate(*frame_pro, *frame_pro, cv::Size(project_shapes.at(mName_)),
                             180, 0, 0, cv::INTER_LINEAR);
        }

		// save result
        sprintf(ad, "%s%s.jpg", mSavePathRaw_.c_str(), t.c_str());
        // std::cout << ad << std::endl;
        if(frame_id % 1 == 0) imwrite(ad, *frame);

        // pass &frame_pro to dequeue
        {
            std::lock_guard<std::mutex> l(mmPro_);
            mPros_->emplace_back(frame_pro);
        }
        delete frame;
        if(frame_id == 10) SetStop();
    }

    timeFile.close();
    cap.release();

}

void VideoCap::SetStop(){
    mbStop_ = true;
}

cv::cuda::GpuMat* VideoCap::GetImage()
{
    cv::cuda::GpuMat* frame;
    while(mPros_->empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    {
        std::lock_guard<std::mutex> lG(mmPro_);
        frame = mPros_->front();
        // std::cout << frame->size() << std::endl;
        mPros_->pop_front();
    }
 
    return frame;
}

void VideoCap::GetUndistortMap(cv::Mat camera_matrix_,
                               cv::Mat dist_coeffs_,
                               cv::Mat scale_xy_,
                               cv::Mat shift_xy_){
    cv::Mat new_camera_matrix = camera_matrix_.clone();
    double* matrix_data = (double *)camera_matrix_.data;
    
    if (!new_camera_matrix.data || !scale_xy_.data || !shift_xy_.data) {
        return;
    }

    const auto scale = (const float *)(scale_xy_.data);
    const auto shift = (const float * )(shift_xy_.data);

    matrix_data[0]         *=  (double)scale[0];
    matrix_data[3 * 1 + 1] *=  (double)scale[1];
    matrix_data[2]         +=  (double)shift[0];
    matrix_data[1 * 3 + 2] +=  (double)shift[1];

    cv::Mat map1, map2;
    cv::fisheye::initUndistortRectifyMap(camera_matrix_, dist_coeffs_, cv::Mat(), new_camera_matrix, mResolution, CV_32F, map1, map2);
    mMap1.upload(map1);
    mMap2.upload(map2);

}
