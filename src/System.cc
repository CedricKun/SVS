#include "System.h"

#include "params.h"


System::System(const char *argv[], size_t num):mConfigPath(argv[1]), mSavePath(argv[2]), mNumViews(num)
{

    if(access(mSavePath.data(), 0)){
        char p[300] = "mkdir -p ";
        std::cout << "raw data save path is not existed\n";
        system(strcat(p, mSavePath.c_str()));
    }
    // std::cout << "System before loady OK" << std::endl;
    LoadWeights();
    
    // std::cout << "System load OK" << std::endl;
    for(size_t i = 0; i < mNumViews; ++i){
        mCaps.emplace_back(new VideoCap(vConfigs[i], vNames[i], mConfigPath, mSavePath));
        // std::thread t = std::thread(&VideoCap::Run, mCaps[i]);
        // t.join();
    }
    std::cout << "System init OK" << std::endl;

}

System::~System(){}

void System::StitchImg()
{   
    std::vector<cv::cuda::GpuMat> views_(4, cv::cuda::GpuMat(mResolution.height, mResolution.width, CV_32FC3));
    std::vector<cv::cuda::GpuMat> out_;
    out_.reserve(mNumViews);

    for (size_t i = 0; i < mCaps.size(); i++)
    {
        out_.emplace_back(new cv::cuda::GpuMat(mWeights[i].size(), CV_32FC3));
    }

    cv::Rect f = cv::Rect(0, 0, total_w, yt);
    cv::Rect l = cv::Rect(0, 0, xl, total_h);
    cv::Rect b = cv::Rect(0, yb, total_w, total_h - yb);
    cv::Rect r = cv::Rect(xr, 0, total_w - xr, total_h);

    int idx = 0;
    char path[300];

    while(1)
    {   
        cv::cuda::GpuMat AVM(total_h, total_w, CV_32FC3, cv::Scalar(0, 0, 0));
        std::cout << "frame id " << idx << " start stitching at: " << ComputeTime() << std::endl;

        for(size_t i = 0; i < mNumViews; ++i){
            cv::cuda::GpuMat v;
            mCaps[i]->GetImage(views_[i]);
            // std::cout << v->size() << std::endl;
            // std::cout << v->size() << std::endl;
            // v.convertTo(v, CV_32FC3);
            // views_[i] = v;
            // cv::Mat tmp;
            // views_[i]->download(tmp);
            // tmp.convertTo(tmp, CV_32FC3, 1.0/255.0);
            // cv::imshow(" ", tmp);
            // cv::waitKey(0);
        }
        
        for (size_t i = 0; i < mNumViews; i++)
        {
            // std::cout << views_[i]->size() << std::endl;
            // std::cout << mWeights[i].size() << std::endl;
            // std::cout << out_[i]->size() << std::endl;
            cv::cuda::multiply(views_[i], mWeights[i], *out_[i]);

        }
        // std::cout << "before add is ok!\n";
        // std::cout << out_[0]->size() << " " << AVM(f).size() << std::endl;
        // std::cout << out_[1]->size() << std::endl;
        // std::cout << AVM(l).size() << std::endl;
        // std::cout << out_[1]->size() << " " << AVM(l).size() << std::endl;
        // std::cout << out_[2]->size() << " " << AVM(b).size() << std::endl;
        // std::cout << out_[3]->size() << " " << AVM(r).size() << std::endl;

        cv::cuda::add(*out_[0], AVM(f), AVM(f));
        cv::cuda::add(*out_[1], AVM(l), AVM(l));
        cv::cuda::add(*out_[2], AVM(b), AVM(b));
        cv::cuda::add(*out_[3], AVM(r), AVM(r));

        AVM.download(mAVM);
        sprintf(path, "%s%d.jpg", mSavePath.c_str(), idx);
        // std::cout << path << std::endl;
        cv::imwrite(path, *mAVM);
        std::cout << "frame id " << idx++ << " finish stitching at: " << ComputeTime() << "\n";

    }    
}

void System::StitchImg(const std::vector<cv::Mat> &photos)
{   
    std::vector<cv::cuda::GpuMat*> views_, out_;
    views_.reserve(mCaps.size());
    out_.reserve(mCaps.size());

    for (size_t i = 0; i < photos.size(); i++)
    {
        cv::Mat p;
        cv::resize(photos[i], p, mWeights[i].size());
        p.convertTo(p, CV_32FC3);
        // views_[i] = new cv::cuda::GpuMat(p);
        views_.emplace_back(new cv::cuda::GpuMat(p));
        out_.emplace_back(new cv::cuda::GpuMat(p.size(), CV_32FC3, cv::Scalar(0, 0, 0)));
    }

    cv::cuda::GpuMat AVM(total_h, total_w, CV_32FC3, cv::Scalar(0, 0, 0));

    cv::Rect f = cv::Rect(0, 0, total_w, yt);
    cv::Rect l = cv::Rect(0, 0, xl, total_h);
    cv::Rect b = cv::Rect(0, yb, total_w, total_h - yb);
    cv::Rect r = cv::Rect(xr, 0, total_w - xr, total_h);

    int idx = 0;
    char path[300];

    std::cout << "before multiply is ok\n";

    for (size_t i = 0; i < views_.size(); i++)
    {
        cv::cuda::multiply(*views_[i], mWeights[i], *out_[i]);
    }
    std::cout << "before add is ok\n";

    std::cout << out_[0]->size() << " " << AVM(f).size() << std::endl;
    std::cout << out_[1]->size() << " " << AVM(l).size() << std::endl;
    std::cout << out_[2]->size() << " " << AVM(b).size() << std::endl;
    std::cout << out_[3]->size() << " " << AVM(r).size() << std::endl;

    cv::cuda::add(*out_[0], AVM(f), AVM(f));
    cv::cuda::add(*out_[1], AVM(l), AVM(l));
    cv::cuda::add(*out_[2], AVM(b), AVM(b));
    cv::cuda::add(*out_[3], AVM(r), AVM(r));
    
    AVM.download(*mAVM);
    sprintf(path, "%s%d.jpg", mSavePath.c_str(), idx++);
    // std::cout << path << std::endl;
    cv::imwrite(path, *mAVM);
   
}

void System::LoadWeights()
{
    std::string weight_path = mConfigPath + "weights.png";
    cv::Mat w_mat = cv::imread(weight_path, -1);
    w_mat.convertTo(w_mat, CV_32F, 1.0/255.0);  
    
    std::cout << "weight load is Ok" << std::endl;
    std::vector<cv::Mat> weights_;
    std::vector<cv::Mat> masks_;

    split(w_mat, weights_);
    std::vector<cv::Mat> w(3);
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            w[j] = weights_[i].clone();
        }
        merge(w, weights_[i]);
    }
    
    std::string mask_path = mConfigPath + "masks.png";
    cv::Mat m_mat = cv::imread(mask_path, -1);
    split(m_mat, masks_);

    std::vector<cv::Mat> whole_weights;
    for (size_t i = 0; i < 2; i++)
    {
        whole_weights.emplace_back(cv::Mat(yt, total_w, CV_32FC3, cv::Scalar(1, 1, 1)));
        whole_weights.emplace_back(cv::Mat(total_h, xl, CV_32FC3, cv::Scalar(1, 1, 1)));
    }

    cv::Mat g1 = whole_weights[0](cv::Rect(0, 0, weights_[2].cols, weights_[2].rows));
    cv::Mat g11 = whole_weights[1](cv::Rect(0, 0, weights_[2].cols, weights_[2].rows));
    addWeighted(g1, 0., weights_[2], 1., 0., g1);
    addWeighted(g11, 1., weights_[2], -1., 0., g11);

    cv::Mat g2 = whole_weights[0](cv::Rect(xr, 0, weights_[1].cols, weights_[1].rows));
    cv::Mat g22 = whole_weights[3](cv::Rect(0, 0, weights_[1].cols, weights_[1].rows));
    addWeighted(g2, 0., weights_[1], 1., 0., g2);
    addWeighted(g22, 1., weights_[1], -1., 0., g22);

    cv::Mat g3 = whole_weights[2](cv::Rect(0, 0, weights_[0].cols, weights_[0].rows));
    cv::Mat g33 = whole_weights[1](cv::Rect(0, yb, weights_[0].cols, weights_[0].rows));
    addWeighted(g3, 0., weights_[0], 1., 0., g3);
    addWeighted(g33, 1., weights_[0], -1., 0., g33);

    cv::Mat g4 = whole_weights[2](cv::Rect(xr, 0, weights_[3].cols, weights_[3].rows));
    cv::Mat g44 = whole_weights[3](cv::Rect(0, yb, weights_[3].cols, weights_[3].rows));
    addWeighted(g4, 0., weights_[3], 1., 0., g4);
    addWeighted(g44, 1., weights_[3], -1., 0., g44);

    for (size_t i = 0; i < 4; i++)
    {
        cv::cuda::GpuMat tmp(whole_weights[i].rows, whole_weights[i].cols, CV_32FC3);
        tmp.upload(whole_weights[i]);
        mWeights.emplace_back(tmp);
        whole_weights[i].convertTo(whole_weights[i], CV_32F, 255.0);
        std::ostringstream ss;
        ss << i << ".jpg";
        cv::imwrite(ss.str(), whole_weights[i]);
    }

}

void System::Start()
{

    while(1){}      // uncomment it if save images
    StitchImg();

}