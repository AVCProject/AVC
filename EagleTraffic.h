/*
	EagleTraffic 1.0 C++ header file
	Copyright (c) 2011 Junho Choi.
*/

#ifndef EAGLETRAFFIC_H
#define EAGLETRAFFIC_H

#include "opencv2/opencv.hpp"
#include <vector>
using namespace std;

// Traffic light values
#define EAGLETRAFFIC_SIGN_NONE		0
#define	EAGLETRAFFIC_SIGN_RED		1
#define EAGLETRAFFIC_SIGN_YELLOW	2
#define EAGLETRAFFIC_SIGN_GREEN		3
#define EAGLETRAFFIC_SIGN_LEFT		11
#define EAGLETRAFFIC_SIGN_RIGHT		12

// Traffic light types
#define EAGLETRAFFIC_LIGHTTYPE_VERTICAL		0
#define EAGLETRAFFIC_LIGHTTYPE_DIRECTION	1

class EagleTraffic {
public:
	EagleTraffic();
	EagleTraffic(int type);
	int decideTrafficLight(const cv::Mat &image); // return traffic light value of the image

private:
	cv::Mat current_frame;
	int _trafficLightType;
};

#endif