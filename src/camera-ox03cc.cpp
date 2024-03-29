#include <stdio.h>  
#include <opencv2/opencv.hpp>  

using namespace cv;  
using namespace std;  

int main(int argc, char** argv)  
{  
    VideoCapture cap("nvv4l2camerasrc device=/dev/video2 ! video/x-raw(memory:NVMM),format=YUY2, width=1920,height=1080,framerate=30/1 ! nvvidconv ! video/x-raw,format=BGRx ! videoconvert ! video/x-raw, format=BGR ! appsink drop=1", cv::CAP_GSTREAMER);
    // VideoCapture cap("v4l2src device=/dev/video7 ! video/x-raw, format=YUY2, width=1920,height=1080,framerate=30/1 ! appsink", cv::CAP_GSTREAMER);
    if (!cap.isOpened())  
    {  
        cout << "Failed to open camera." << endl;  
        return -1;  
    }  
    int d=0;
    char ad[300]={0};
    // double prev = (double) cv::getTickCount();

    while(cap.isOpened())
    {  
        Mat frame;  
        cap >> frame; 
        // cout << frame.size() << endl;
        // cvtColor(frame, frame, COLOR_YUV2BGR_YUYV);
        // double cur = (double) cv::getTickCount();
		// double delta = (cur - prev) / cv::getTickFrequency();
        // prev = cur;

        imshow("original", frame);
        int kk=waitKey(1); 
        if(kk=='q')
        {
            break;
        }
        if(kk=='p')
        {
            sprintf(ad, "/Parking/AVM/pics/2/%d.jpg", ++d);
            imwrite(ad, frame);
            cout<<"OK " <<endl;
        }
    }  
    cap.release();
    return 0;  
}



