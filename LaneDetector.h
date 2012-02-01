/*
 *  LaneDetector.h
 *  AVC
 *
 *  Created by YoungJae Kwon on 11. 8. 11..
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "opencv2/opencv.hpp"
#include <list>

using namespace cv;
using namespace std;

typedef enum 
{
	LaneDetectorTrackBarBinaryThreshold,
	LaneDetectorTrackBarBinaryThresholdType,
	LaneDetectorTrackBarCannyParam,
	LaneDetectorTrackBarHoughThreshold,
	LaneDetectorTrackBarRMin,
	LaneDetectorTrackBarGMin,
	LaneDetectorTrackBarBMin,
	LaneDetectorTrackBarBMax,
	LaneDetectorTrackBarErosionType,
	LaneDetectorTrackBarErosionSize,
	LaneDetectorTrackBarDialationType,
	LaneDetectorTrackBarDialationSize
} LaneDetectorTrackBarType;

class LaneDetector
{
public:

	LaneDetector(const char* aWindowName);
	~LaneDetector();
	void runModule(Mat &frame,cv::Rect roiRect);

	void saveParamenters();
	void erosion( Mat &frame, Mat &result );
	void dilation( Mat &frame, Mat &result );
	
	void eliminateOutLiers(vector<Vec2f>& aLines);
	bool getAverageLine(vector<Vec2f>& aLines, Vec2f& avgLine);
	int drawDetectedLine(Vec2f& aLine, Mat& bgImage,Scalar color);

	void detectFromBinaryImage(Mat& g_targetImage, Mat& frameToShow, Scalar color,std::list<Vec2f>& rightLineHistory,std::list<Vec2f>& leftLineHistory, Vec2f& rightLineHistorySum, Vec2f& leftLineHistorySum );
	
	int convertPixelToDistance(int aPixel);
    
	// 외부 액세스
	float angleLeft;
	float angleRight;
    float lateralOffset;
    float leftMargin;
    float rightMargin;
    
    float pixToDistRatio;
protected:

    cv::Size houghROI;
    
	std::list<Vec2f> R_Yellow_LaneHistory;
	std::list<Vec2f> L_Yellow_LaneHistory;
	
	Vec2f R_Yellow_LaneHistorySum;
	Vec2f L_Yellow_LaneHistorySum;
	

	std::list<Vec2f> R_LaneHistory;
	std::list<Vec2f> L_LaneHistory;
	
	Vec2f R_LaneHistorySum;
	Vec2f L_LaneHistorySum;

	
	int maxNumberOfHistory;
	string windowName;
	string winName_hough;
	string winName_binary;
	string winName_trackbar;
	string winName_original;

	// 이진화에 사용될 문턱값 및 트랙바
	LaneDetectorTrackBarType barTypeBT;
	LaneDetectorTrackBarType barTypeBTT;	
	LaneDetectorTrackBarType barTypeHough;
	
	LaneDetectorTrackBarType barTypeRMin;
	LaneDetectorTrackBarType barTypeGMin;
	LaneDetectorTrackBarType barTypeBMin;
	LaneDetectorTrackBarType barTypeBMax;

	LaneDetectorTrackBarType barTypeErosionType;
	LaneDetectorTrackBarType barTypeErosionSize;
	
	LaneDetectorTrackBarType barTypeDilationType;
	LaneDetectorTrackBarType barTypeDilationSize;
	

	
};