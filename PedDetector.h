//
//  PedDetector.h
//  AVC
//
//  Created by YoungJae Kwon on 12. 1. 10..
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef AVC_PedDetector_h
#define AVC_PedDetector_h

#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

class PedDetector
{
public:
    
	PedDetector(const char* aWindowName);
	~PedDetector();
	void runModule(Mat &frame,cv::Rect roiRect);
    
protected:
    string windowName;
    
    HOGDescriptor hog;    
};

#endif
