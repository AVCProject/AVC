/*
	EagleCrosswalk 1.0 C++ header file
	Copyright (c) 2012 Junho Choi.
*/

#ifndef EAGLECROSSWALK_H
#define EAGLECROSSWALK_H

#include "opencv2/opencv.hpp"
#include <vector>
using namespace std;

class EagleCrosswalk {
public:
	EagleCrosswalk();
	int decideCrosswalk(const cv::Mat &image); // return crosswalk vertical position of the image

private:
	cv::Mat current_frame;
	int previous_pos;
	int previous_value;
};

#endif