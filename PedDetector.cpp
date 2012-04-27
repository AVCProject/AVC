//
//  PedDetector.cpp
//  AVC
//
//  Created by YoungJae Kwon on 12. 1. 10..
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

// 디버그 모드: 결과값 콘솔에 출력
#define PED_DEBUG 1

#include "PedDetector.h"
#include "AVCTimeProfiler.h"
#include <iostream>

// static일경우 컴파일타임에 모두 결정되므로 스태틱 바인딩
// extern일경우 각 파일의 컴파일의 obj파일 생성 후 링킹타임에 다이나믹 바인딩(자동으로 다른 obj파일의 전역변수를 찾아서 바인딩 해준다.)
extern int g_pushedKey; // global.h에 정의되어 있지만 global.h를 직접적으로 컴파일하기위해 포함할 필요는 없음.

using namespace cv;
using namespace std;

static PedDetector* g_pedDetector = NULL;

// 오픈씨비 이벤트 함수포인터에 멤버변수를 직접 넣을 수 없어서 우회하는 함수
void onMouse_global( int event, int x, int y, int flag, void* func )
{
	if(g_pedDetector != NULL)
		g_pedDetector->onMouse(event,x,y,flag,func);
}

void PedDetector::onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )
    {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);
        
       // selection &= cv::Rect(0, 0, image.cols, image.rows);
    }
    
    switch( event )
    {
        case CV_EVENT_LBUTTONDOWN:
            origin = cv::Point(x,y);
            selection = cv::Rect(x,y,0,0);
            selectObject = true;
            break;
        case CV_EVENT_LBUTTONUP:
            selectObject = false;
            if( selection.width > 0 && selection.height > 0 )
                trackObject = -1;
            break;
    }
}


PedDetector::PedDetector(const char* aWindowName)
{
	// 외부 XML 혹은 UI 형태로 빼놓을것.
	param_scaleStepSize = 1.03;
	param_colorPixelPercentTh = 0.05;
	param_pedestrianLargeTh = -1.8;
	param_pedestrianSmallTh = 0;
	
	param_colorPixelRange.R_min = 160;	param_colorPixelRange.R_max = 224;
	param_colorPixelRange.G_min = 57;	param_colorPixelRange.G_max = 80;
	param_colorPixelRange.B_min = 30;	param_colorPixelRange.B_max = 54;

	param_modeChangeTh = 8;

	windowName = aWindowName;

	namedWindow( aWindowName, 0 );
	namedWindow( "Histogram", CV_WINDOW_AUTOSIZE );
    setMouseCallback( aWindowName, onMouse_global, 0 );

	// 캠시프트 사용한 트랙킹에 필요한 파라메터.
    //  createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
    //  createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
    //  createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );
    vmin = 10, vmax = 256, smin = 30;


    

	// onMouse_global 전역함수 사용위해
	g_pedDetector = this;

	
	
	// 백프로젝션 관련 변수 설정
	backprojMode = false;
	selectObject = false;
	trackObject = 0;
	showHist = true;

	histimg = Mat::zeros(200, 320, CV_8UC3);
	// HSV에서 HS 성분만 갖고 히스토그램 생성 후 비교. 다른 색상영역에서 비교 실험도 필요함.
    // 0 ~ 180사이의 hue값을 레벨당 16만큼 양자화	
	hueQtzSize = 16;
	satQtzSize = 16;
	histQtzs[0] = hueQtzSize;
	histQtzs[1] = satQtzSize;
    
    // hue varies from 0 to 179, see cvtColor
    hueRanges[0] = 0;
	hueRanges[1] = 180;
 
	satRanges[0] = 0;
	satRanges[1] = 255;

	histChannel[0] = 0;
	histChannel[1] = 1;

	histRanges[0] = hueRanges;
	histRanges[1] = satRanges;

	isFound = false;
	pedContinuousNotFoundCnt = 0;
	hog = NULL;

	setModeTo48x96();

#ifdef MAC_OS_X_VERSION_10_7
    hogLarge = new HOGDescriptor();
	hogLarge->setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
#else

	
// 	hogLarge = new gpu::HOGDescriptor();
// 	hogLarge->setSVMDetector(gpu::HOGDescriptor::getDefaultPeopleDetector());

	// OpenCV 버그?: 두개의 HOG 디스크립터를 선언할 경우 꼬여서 디텍션이 안되는 현상 발생. 하나만 사용할것
	// GPU처리 함수 관련해서 두개가 동시 upload가 안되는건지 모르겠음.

// 	hogSmall = new gpu::HOGDescriptor(Size(48, 96), // peopleDetector의 사이즈와 일치되게 할것
// 		Size(16, 16),Size(8,8), Size(8,8),9,-1, // 설정 불가능한 파라메터들
// 		0.2, true, 64); // double threshold_L2hys=0.2, bool gamma_correction=true, int nlevels=DEFAULT_NLEVELS
// 	hogSmall->setSVMDetector(gpu::HOGDescriptor::getPeopleDetector48x96());

#endif  
}

void PedDetector::setModeTo48x96()
{
#ifdef PED_DEBUG
	cout << "[Mode changed to 48x96]" << endl;
#endif
	isMode48x96 = true;
	if(hog)
		delete hog;

	hog = new gpu::HOGDescriptor(Size(48, 96), // peopleDetector의 사이즈와 일치되게 할것
		Size(16, 16),Size(8,8), Size(8,8),9,-1, // 설정 불가능한 파라메터들
		0.2, true, 64); // double threshold_L2hys=0.2, bool gamma_correction=true, int nlevels=DEFAULT_NLEVELS
	hog->setSVMDetector(gpu::HOGDescriptor::getPeopleDetector48x96());
}

void PedDetector::setModeTo64x128()
{
#ifdef PED_DEBUG
	cout << "[Mode changed to 64x128]" << endl;
#endif
	isMode48x96 = false;
	if(hog)
		delete hog;

	hog = new gpu::HOGDescriptor();
	hog->setSVMDetector(gpu::HOGDescriptor::getDefaultPeopleDetector());

}

void PedDetector::runModule(Mat &frame, cv::Rect roiRect)
{
	cv::Mat ROI(frame, roiRect);

	/*
	int key = g_pushedKey;
	if( key == 'b')
		backprojMode = !backprojMode;

    
	//캠시프트 시작
	
	cvtColor(ROI, hsv, CV_BGR2HSV);
	if( trackObject )
	{
		// 채도만 갖고 추출할경우 사용되는 코드.
		//int _vmin = vmin, _vmax = vmax;
		
		//code:마스크ON
		// 해당 범위내에 들어오면 마스크에 binary 표시
		//inRange(hsv, Scalar(0,  smin, MIN(_vmin, _vmax)), 
		//			 Scalar(180, 256, MAX(_vmin, _vmax)), mask);
		
		//Convert the original to HSV format and separate only Hue channel to be used for the Histogram
		//int ch[] = {0, 0};
		//hue.create(hsv.size(), hsv.depth());
		//mixChannels(&hsv, 1, &hue, 1, ch, 1); // hsv{0,1,2} channel 중 0번채널을 -> hue{0} channel로 복사.
		
	
		// 새로운 영역이 마우스에 의해서 선택되었을 경우 히스토그램 새로 추출
		if( trackObject < 0 )
		{
			trackWindow = selection;
			trackObject = 1;

			// hue를 가진 매트릭스에서 select된부분만을 새로 뽑아냄(Region Of Interest)
			Mat selectedArea(hsv, selection);
				
			//code:마스크ON
			//Mat maskroi(mask, selection);
			
			//code:마스크ON
			//calcHist(&selectedArea, 1, histChannel, mask, hist, 2, histQtzs, (const float **)histRanges);

			calcHist(&selectedArea, 1, histChannel, Mat(), hist, 2, histQtzs, (const float **)histRanges);


			normalize(hist, hist, 0, 255, CV_MINMAX);
			int scale = 10;
			double maxVal=0;
			minMaxLoc(hist, 0, &maxVal, 0, 0);

			Mat histImg2 = Mat::zeros(satQtzSize*scale, hueQtzSize*10, CV_8UC3);

			for( int h = 0; h < hueQtzSize; h++ )
			{
				for( int s = 0; s < satQtzSize; s++ )
				{
					float binVal = hist.at<float>(h, s);
					int intensity = cvRound(binVal*255/maxVal);
					rectangle( histImg2, Point(h*scale, s*scale),
						Point( (h+1)*scale - 1, (s+1)*scale - 1),
						Scalar::all(intensity),
						CV_FILLED );
				}
			}
			imshow( "Histogram", histImg2 );
		}

		// hue 이미지를 히스토그램 backprojection하여 face probability 이미지 생성(변수명 backproj)
		// Similarly to calcHist , at each location (x, y) the function collects the values from the selected 
		// channels in the input images and finds the corresponding histogram bin. 
		// But instead of incrementing it, the function reads the bin value, scales it by scale and stores in backProject(x,y)

		
		calcBackProject(&hsv, 1, histChannel, hist, backproj, (const float**)histRanges);

		//code:마스크ON
		//backproj &= mask;

		// CAM(Continuously Adaptive Mean) shift 
		// It finds this new location by starting at the previous location and computing the center of gravity of the face-probability values within a rectangle.
		// It then shifts the rectangle so it's right over the center of gravity

		// CamShift( 얼굴확률 이미지, 트랙킹할 렉트, 조건 )
		RotatedRect trackBox = CamShift(backproj, trackWindow,TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));

		// backproject된  face probability를 이미지 형태로 출력
		if( backprojMode )
			cvtColor( backproj, ROI, CV_GRAY2BGR );

		// 오브젝트 트래킹 타원 그리기
		ellipse( ROI, trackBox, Scalar(0,0,255), 3, CV_AA );
	}

	// 마우스 선택영역 반전
	if( selectObject && selection.width > 0 && selection.height > 0 )
	{
		Mat roi(ROI, selection);
		bitwise_not(roi, roi);
	}
	//imshow( "Histogram", histimg );

	// 캠시프트 끝
	*/

   	vector<cv::Rect> found, found_filtered;
	
	// run the detector with default parameters. to get a higher hit-rate
	// (and more false alarms, respectively), decrease the hitThreshold and
	// groupThreshold (set groupThreshold to 0 to turn off the grouping completely).
		
	cv::gpu::GpuMat gpuSrcRGB, gpuSrcGray;
    gpuSrcRGB.upload(ROI);
	gpu::cvtColor(gpuSrcRGB,gpuSrcGray,CV_RGB2GRAY);

	//hit_threshold ? Threshold for the distance between features and SVM classifying plane.
	//Usually it is 0 and should be specfied in the detector coefficients (as the last free coefficient).
	//But if the free coefficient is omitted (which is allowed), you can specify it manually here.

	//48x96 setting
	//SVM 문턱값을 높이되, ->너무 높으면 정면보행자가 아니라 디텍션 불가 1.4 정도?
	//멀티스케일 계수를 적절히 조정할것 1.03정도면 30~40ms
	//hogSmall->detectMultiScale(gpuSrcGray, found, param_pedestrianSmallTh, cv::Size(8,8), cv::Size(0,0), param_scaleStepSize, 2);

	//64x128 setting
	// 0보다 작은 문턱값을 사용해야 어느정도 디텍션이 가능해짐.
	// 윈도우 사이즈 16x16으로 더크게하면 정확도는 떨어지지만 속도 향상.
	//hogLarge->detectMultiScale(gpuSrcGray, found, param_pedestrianLargeTh, cv::Size(8,8), cv::Size(0,0), param_scaleStepSize, 2);

	if(isMode48x96)
		hog->detectMultiScale(gpuSrcGray, found, param_pedestrianSmallTh, cv::Size(8,8), cv::Size(0,0), param_scaleStepSize, 2);
	else
		hog->detectMultiScale(gpuSrcGray, found, param_pedestrianLargeTh, cv::Size(8,8), cv::Size(0,0), param_scaleStepSize, 2);

	
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

	cv::Rect r;
	for( i = 0; i < found_filtered.size(); i++ )
	{
		r = found_filtered[i];

		// the HOG detector returns slightly larger rectangles than the real objects.
		// so we slightly shrink the rectangles to get  a nicer output.
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);

		
		/*
		// 컬러 히스토그램 백프로젝션을 이용한 걸러내기: 성능이 그렇게 좋지는 않다.
		double probThreshold = 0.09;
		double prob = 0;
		if( backproj.rows != 0 && backproj.cols != 0 )
		{
			Mat humanROI(backproj,r);
			cv::Scalar sum = cv::sum(humanROI);
			double* vals = sum.val;
			prob = (vals[0] / (r.width*r.height)) / 255;
			cout << "sum:" << prob << endl;
		}

		if( probThreshold < prob )
			rectangle(ROI, r.tl(), r.br(), cv::Scalar(0,255,0), 3);
		else
			rectangle(ROI, r.tl(), r.br(), cv::Scalar(255,0,0), 3);
*/

		// 찾아진 사람의 상체 옷영역에 대한 컬러 정보를 이용해서 결과를 필터링 
		cv::Rect upperRect;

		upperRect.x = r.x + r.width/3;
		upperRect.y = r.y + r.height/4;
		upperRect.width = r.width*2/3;
		upperRect.height = r.height*2/4;

		Mat upperBodyROI(ROI,upperRect);

		vector<Mat> planes;
		split(upperBodyROI, planes);

		// 옷색깔을 가진 픽셀 수 카운팅
		int redCnt = 0;
		for(int i = 0; i < upperBodyROI.rows ; i++)
		{
			// rgb 순서주의
			uchar* bb = planes[0].ptr<uchar>(i);
			uchar* gg = planes[1].ptr<uchar>(i);
			uchar* rr = planes[2].ptr<uchar>(i);	

			for(int x = 0; x < upperBodyROI.cols ; x++)
			{
				if( param_colorPixelRange.R_min < rr[x] && rr[x] < param_colorPixelRange.R_max &&
					param_colorPixelRange.G_min < gg[x] && gg[x] < param_colorPixelRange.G_max &&
					param_colorPixelRange.B_min < bb[x] && bb[x] < param_colorPixelRange.B_max)
				{
					redCnt++;
				}
			}
		}

		// 0.1 퍼센트 이상의 픽셀이 컬러에 맞으면 true match 아닐경우 false match
		if( redCnt > upperBodyROI.rows*upperBodyROI.cols* param_colorPixelPercentTh)
		{
			rectangle(ROI, r.tl(), r.br(), cv::Scalar(0,255,0), 3);
			isFound = true;
		}
		else
		{
			rectangle(ROI, r.tl(), r.br(), cv::Scalar(255,0,0), 3);
		}

		// 상체 부분 upperROI 박스 그리기
		if(isMode48x96)
			rectangle(ROI, upperRect.tl(), upperRect.br(), cv::Scalar(255,255,255), 2);
		else
			rectangle(ROI, upperRect.tl(), upperRect.br(), cv::Scalar(128,128,128), 2);
	}
   
	if (isFound)
	{
		if( isMode48x96 && r.width > 58 )
			setModeTo64x128();

		#ifdef PED_DEBUG
		cout << "Ped: O" << " rect:" << r.width << "," << r.height << endl;
		#endif
		pedContinuousNotFoundCnt = 0;
	}
	else
	{
		#ifdef PED_DEBUG
		cout << "Ped: X" << endl;
		#endif
		pedContinuousNotFoundCnt++;
		
		// 64x128모드에서 일정프레임 이상 사람이 디텍션이 안될경우 다시 48x96모드로 전환
		if( !isMode48x96 && (pedContinuousNotFoundCnt > param_modeChangeTh) )
			setModeTo48x96();
	}

	isFound = false;
    
    imshow(windowName, ROI);
	
}

PedDetector::~PedDetector()
{
	if(hog)
		delete hog;
}