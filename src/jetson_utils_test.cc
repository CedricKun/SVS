// #include <iostream>
// #include <vector>

// #include <jetson-utils/videoSource.h>
// #include <jetson-utils/videoOutput.h>
// #include <jetson-utils/imageFormat.h>

// #include <opencv2/opencv.hpp>
// #include "opencv2/cudaarithm.hpp"
// #include "opencv2/cudaimgproc.hpp" 


// int main(int argc, char **argv) {

// 	// create input stream
// 	videoOptions opt;
// 	// opt.codecType = videoOptions::CODEC_V4L2;
// 	opt.width  = 1920;
// 	opt.height = 1080;
// 	opt.frameRate = 10;
// 	opt.zeroCopy = false; // GPU access only for better speed
// 	if(argc != 2)
// 	{
// 		std::cerr << "Error: Invalid argv to create camera opt" << std::endl;
// 		exit(1);
// 	}
// 	videoSource * input = videoSource::Create("csi://0", opt);
// 	if (!input) {
// 		std::cerr << "Error: Failed to create input stream" << std::endl;
// 		exit(-1);
// 	}
// 	input->GetOptions().Print();


// 	// create output stream
// 	videoOutput* output = videoOutput::Create("display://0");
// 	if( !output ) {
// 		std::cerr << "Error: Failed to create output stream" << std::endl;
// 		delete input;
// 		exit(-2);
// 	}

// 	// Read one frame to get resolution
// 	float3* image = NULL;
// 	if( !input->Capture(&image) )
// 	{
// 		std::cout << "****************************\n";
// 		std::cerr << "Error: failed to capture first video frame" << std::endl;
// 		delete output;
// 		delete input;
// 		exit(3);
// 	}


// 	/*
// 	 * processing loop
// 	 */
// 	cv::cuda::GpuMat g_bgr(input->GetHeight(), input->GetWidth(), CV_32FC3);
// 	double prev = (double) cv::getTickCount();
// 	while( 1 )
// 	{
// 		// capture next image
// 		if( !input->Capture(&image, 1000) )
// 		{
// 			std::cerr << "Error: failed to capture video frame" << std::endl;
// 			continue;
// 		}
// 		// log timing
// 		double cur = (double) cv::getTickCount();
// 		double delta = (cur - prev) / cv::getTickFrequency();
// 		std::cout<<"delta=" << delta << std::endl;
// 		prev=cur;

// 		// Some OpenCv processing
// 		cv::cuda::GpuMat frame_in(input->GetHeight(), input->GetWidth(), CV_32FC3, image);
// 		// cv::cuda::cvtColor(frame_in, g_bgr, cv::COLOR_BGR2RGB);
// 		// cv::cuda::remap();
// 		// cv::cuda::warpPerspective();

// 		// Display result
// 		output->Render((float3*)frame_in.data, input->GetWidth(), input->GetHeight());
// 		if( !output->IsStreaming() )
// 			break;
// 		if( !input->IsStreaming() )
// 			break;
// 	}

// 	delete input;
// 	delete output;
//    	return 0;
// }
