//
//  RoadAreaDetector.cpp
//  AVC
//
//  Created by YoungJae Kwon on 12. 1. 10..
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include "RoadAreaDetector.h"
#include <iostream>

static int global_x = 0;
static int global_y = 0;

void onMouse( int event, int x, int y, int, void* )
{
    if( event != CV_EVENT_LBUTTONDOWN )
        return;
    
    global_x = x;
    global_y = y;
    cout << x << " " << y << endl;
}

using namespace cv;
using namespace std;

RoadAreaDetector::RoadAreaDetector(const char* aWindowName)
{
	windowName = aWindowName;
    
    /*
    cout << "Simple floodfill mode is set\n";
    ffillMode = 0;
    
    cout << "Fixed Range floodfill mode is set\n";
    ffillMode = 1;
    
    cout << "Gradient (floating range) floodfill mode is set\n";
    ffillMode = 2;
    
    
     cout << "4-connectivity mode is set\n";
    connectivity = 4;
    
    cout << "8-connectivity mode is set\n";
    connectivity = 8;
*/
    
    
    ffillMode = 1;
    loDiff = 4, upDiff = 4;
    connectivity = 4;
    isColor = true;
    useMask = false;
    newMaskVal = 255;
    
    namedWindow( aWindowName, 0 );
    createTrackbar( "lo_diff", aWindowName, &loDiff, 255, 0 );
    createTrackbar( "up_diff", aWindowName, &upDiff, 255, 0 );
    
    setMouseCallback( aWindowName, onMouse, 0 );
}

void RoadAreaDetector::runModule(Mat &frame, cv::Rect roiRect)
{
    cvtColor(frame, gray, CV_BGR2GRAY); 
   
    //cv::Point seed =  roiRect.tl();
    cv::Point seed =  cv::Point(global_x,global_y);
    int lo = ffillMode == 0 ? 0 : loDiff;
    int up = ffillMode == 0 ? 0 : upDiff;
    int flags = connectivity + (newMaskVal << 8) +
    (ffillMode == 1 ? CV_FLOODFILL_FIXED_RANGE : 0);
    int b = (unsigned)theRNG() & 255;
    int g = (unsigned)theRNG() & 255;
    int r = (unsigned)theRNG() & 255;
    cv::Rect ccomp;
    
    Scalar newVal = isColor ? Scalar(b, g, r) : Scalar(r*0.299 + g*0.587 + b*0.114);
    Mat dst = isColor ? frame : gray;
    int area;
    
    if( useMask )
    {
        threshold(mask, mask, 1, 128, CV_THRESH_BINARY);
        area = floodFill(dst, mask, seed, newVal, &ccomp, Scalar(lo, lo, lo),
                         Scalar(up, up, up), flags);
        imshow( "mask", mask );
    }
    else
    {
        area = floodFill(dst, seed, newVal, &ccomp, Scalar(lo, lo, lo),
                         Scalar(up, up, up), flags);
    }
    
    imshow(windowName, dst);
    cout << area << " pixels were repainted\n";
    
}

RoadAreaDetector::~RoadAreaDetector()
{

}