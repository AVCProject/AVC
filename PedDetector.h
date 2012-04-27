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

	// MacType.h�� Rect,Point�� OpenCV�� �浹�Ͼ ��������� �����ٰ�
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

	// 0 ~ 180������ hue���� ������ 16��ŭ ����ȭ
	int histQtzSize;

	int hueQtzSize;
	int satQtzSize;
	int histQtzs[2];

	float hueRanges[2];
	float satRanges[2];
	float* histRanges[2];

	int histChannel[2];

	int pedContinuousNotFoundCnt;	// �����ڰ� �������� �߰ߵ��� ���� ī��Ʈ
	bool isMode48x96;				// ���� ��ũ���� ���� ���
	bool isFound;					// ���� �����ӿ��� ����� �߰ߵǾ����� ����

	// �Ķ���� ������ ����
	float param_colorPixelPercentTh;	// ã���� �����ڿ��� �ʺκ��� �÷����� ���ΰ�
	float param_pedestrianLargeTh;		// 64x128 �� ���� ������ ���ؼ� ���ΰ� 
	float param_pedestrianSmallTh;		// 48x96 �� ���� ������ ���ؼ� ���ΰ�	
	float param_scaleStepSize;			// ��Ƽ������ ���ؼ� �Ҷ� ������ ũ�� ������Ű�� ���ܻ�����
	RGBRange param_colorPixelRange;

	int param_modeChangeTh;				// �����ڰ� �������� ��ȸ�̻� �߰ߵ��� �ʴ´ٸ� 48x96���� �ٽ���ȯ
	

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
