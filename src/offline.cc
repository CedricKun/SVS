#include "common.h"

#include "params.h"
#include "VideoCap.h"
#include "System.h"

using namespace std;
using namespace cv;

unsigned num_threads = sysconf(_SC_NPROCESSORS_CONF);;

int main(int argc, const char *argv[])
{
    // if(argc < 3 || argc > 4){
    //     std::cerr << std::endl
    //          << "Usage: ./bin/run_avm path_to_yaml path_to_save_path"
    //          << std::endl;
    //     return 1;
    // }
    // size_t num_view = 4;

    // System *AVM = new System(argv, num_view);

    // std::vector<cv::Mat> photo;
    // std::string path = "/home/nvidia/data/Parking/AVM/photo/";
    // std::string view[4] = {"front", "left", "back", "right"};
    // for (size_t i = 0; i < num_view; i++)
    // {
    //     std::string p = path + view[i] + ".jpg";
    //     cv::Mat img = cv::imread(p, -1);
    //     photo.emplace_back(img);
    // }
    // AVM->StitchImg(photo);

    std::string path = "/media/nvidia/19ecd9bf-0f9c-49d0-a5a4-6e271d5a3613/Parking/AVM/build/0.jpg";
    cv::Mat img = cv::imread(path, -1);
    cv::cuda::GpuMat gmat;
    gmat.upload(img);
    cv::cuda::rotate(gmat, gmat, cv::Size(img.size().height, img.size().width), 90, 0, img.size().width);
    gmat.download(img);
    cv::imwrite("out.jpg", img);
    // cv::imshow("", img);
    // cv::waitKey(0);



    return 0;
}

