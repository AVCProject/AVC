//
//  RoadAreaDetector.h
//  AVC
//
//  Created by YoungJae Kwon on 12. 1. 10..
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef AVC_RoadAreaDetector_h
#define AVC_RoadAreaDetector_h

#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

class RoadAreaDetector
{
public:
    
	RoadAreaDetector(const char* aWindowName);
	~RoadAreaDetector();
	void runModule(Mat &frame,cv::Rect roiRect);
    
protected:
    string windowName;
    
    Mat gray, mask;
    int ffillMode;
    int loDiff, upDiff;
    int connectivity;
    int isColor;
    bool useMask;
    int newMaskVal;
    
};

#endif
