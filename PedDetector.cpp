//
//  PedDetector.cpp
//  AVC
//
//  Created by YoungJae Kwon on 12. 1. 10..
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include "PedDetector.h"
#include <iostream>

using namespace cv;
using namespace std;

PedDetector::PedDetector(const char* aWindowName)
{
	windowName = aWindowName;

	//hog.setSVMDetector(gpu::HOGDescriptor::getDefaultPeopleDetector());
    hog.setSVMDetector(gpu::HOGDescriptor::getPeopleDetector64x128());
  
    namedWindow( aWindowName, 0 );
    
    //createTrackbar( "lo_diff", aWindowName, &loDiff, 255, 0 );
    //createTrackbar( "up_diff", aWindowName, &upDiff, 255, 0 );
   // setMouseCallback( aWindowName, onMouse, 0 );
}

void PedDetector::runModule(Mat &frame, cv::Rect roiRect)
{
    cv::Mat ROI(frame, roiRect);
    
   	vector<cv::Rect> found, found_filtered;
	double t = (double)getTickCount();
	// run the detector with default parameters. to get a higher hit-rate
	// (and more false alarms, respectively), decrease the hitThreshold and
	// groupThreshold (set groupThreshold to 0 to turn off the grouping completely).

		
	cv::gpu::GpuMat gpuSrcRGB, gpuSrcGray;
    gpuSrcRGB.upload(ROI);
	gpu::cvtColor(gpuSrcRGB,gpuSrcGray,CV_RGB2GRAY);

    //hog.detectMultiScale(gpuSrcGray, found, -0.3, cv::Size(8,8), cv::Size(0,0), 1.05, 2);
	hog.detectMultiScale(gpuSrcGray, found, -0.3, cv::Size(8,8), cv::Size(0,0), 1.03, 2);
    
	t = (double)getTickCount() - t;
	printf("tdetection time = %gms\n", t*1000./cv::getTickFrequency());
	size_t i, j;
	for( i = 0; i < found.size(); i++ )
	{
		cv::Rect r = found[i];
		for( j = 0; j < found.size(); j++ )
			if( j != i && (r & found[j]) == r)
				break;
		if( j == found.size() )
			found_filtered.push_back(r);
	}
	for( i = 0; i < found_filtered.size(); i++ )
	{
		cv::Rect r = found_filtered[i];
		// the HOG detector returns slightly larger rectangles than the real objects.
		// so we slightly shrink the rectangles to get a nicer output.
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);
		rectangle(ROI, r.tl(), r.br(), cv::Scalar(0,255,0), 3);
	}
   
    
    imshow(windowName, ROI);
   
}

PedDetector::~PedDetector()
{

}