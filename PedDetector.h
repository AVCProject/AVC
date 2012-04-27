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
#include "opencv2/gpu/gpu.hpp"

using namespace cv;
using namespace std;

typedef struct 
{
	uchar R_min;
	uchar R_max;
	uchar G_min;
	uchar G_max;
	uchar B_min;
	uchar B_max;

} RGBRange;

class PedDetector
{
public:
    

	PedDetector(const char* aWindowName);
	~PedDetector();

	void onMouse( int event, int x, int y, int, void* );
	void runModule(Mat &frame,cv::Rect roiRect);
    
protected:
    string windowName;

	// MacType.h의 Rect,Point와 OpenCV와 충돌일어남 명시적으로 적어줄것
	Point2i origin;
	cv::Rect selection;

	int vmin, vmax, smin;
	Mat image;

	bool backprojMode;
	bool selectObject;
	int trackObject;
	bool showHist;

	Mat hsv, hue, mask, hist, histimg, backproj;

	cv::Rect trackWindow;
	RotatedRect trackBox;

	// 0 ~ 180사이의 hue값을 레벨당 16만큼 양자화
	int histQtzSize;

	int hueQtzSize;
	int satQtzSize;
	int histQtzs[2];

	float hueRanges[2];
	float satRanges[2];
	float* histRanges[2];

	int histChannel[2];

	int pedContinuousNotFoundCnt;	// 보행자가 연속으로 발견되지 않은 카운트
	bool isMode48x96;				// 현재 디스크립터 동작 모드
	bool isFound;					// 현재 프레임에서 사람이 발견되었는지 여부

	// 파라메터 설정값 모음
	float param_colorPixelPercentTh;	// 찾아진 보행자에서 옷부분의 컬러비율 문턱값
	float param_pedestrianLargeTh;		// 64x128 값 사용시 보행자 디텍션 문턱값 
	float param_pedestrianSmallTh;		// 48x96 값 사용시 보행자 디텍션 문턱값	
	float param_scaleStepSize;			// 멀티스케일 디텍션 할때 윈도우 크기 증가시키는 스텝사이즈
	RGBRange param_colorPixelRange;

	int param_modeChangeTh;				// 보행자가 연속으로 몇회이상 발견되지 않는다면 48x96모드로 다시전환
	

	void setModeTo48x96();
	void setModeTo64x128();

#ifdef MAC_OS_X_VERSION_10_7
	HOGDescriptor* hogLarge;    
#else
    gpu::HOGDescriptor* hogSmall;  
	 gpu::HOGDescriptor* hogLarge;  
	 gpu::HOGDescriptor* hog;  
#endif
};

#endif
