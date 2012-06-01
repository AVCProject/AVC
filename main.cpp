/*
 *  main.cpp
 *  AVC
 *
 *  Created by YoungJae Kwon on 11. 8. 11..
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */



#include "opencv2/opencv.hpp"
#include "boost/thread.hpp"
#include <cassert>
#include <iostream>

//#include <ctype.h>
#include "global.h"

#include "RoadAreaDetector.h"
#include "LaneDetector.h"
#include "PedDetector.h"
#include "EagleTraffic.h"
#include "EagleCrosswalk.h"

#include "AVCTimeProfiler.h"
#include "AVCNetwork.h" // 인클루드 순서가 중요하다. opencv_gpu모듈이 인클루드 된 후에 적어줄것.

using namespace cv;
using namespace std;



#ifdef MAC_OS_X_VERSION_10_7
string getFilePathFromBundle(const char* aName);
#endif


void getBirdEyeView(Mat& src);

int main (int argc, char * const argv[]) 
{    
	bool isPaused = false;
	bool isPausedWithLastFrame = false;
	bool isNetworkOn = true;
    bool isSourceLive = false;
	//char filePath[] = "curve_test.avi";
    //char filePath[] = "C:\\AVCData\\1322203220.avi";
    char filePath[] = "c:\\AVC_Output\\AVC_Data\\b2.avi";
	AVCNetwork *netModule = NULL;
    
	if (isNetworkOn) 
	{
        netModule = new AVCNetwork();
        netModule->init(6000);
        netModule->start();
	}
    
    
	VideoCapture cap;
    
    if (isSourceLive) 
    {
        // 카메라 라이브 로딩
        cap.open(0);
    }
    else
    {
        cap.open( filePath );   
    }
	
	// 저장된 비디오에서 로딩
	
    
    if( !cap.isOpened() )
    {
        cout << "***Could not initialize capturing...***\n";
        return 0;
    }
	
	//LaneDetector *laneDetector = new LaneDetector("LaneDetector");
    //RoadAreaDetector *roadAreaDetector = new RoadAreaDetector("RoadAreaDetector");
    PedDetector *pedDetector = new PedDetector("PedDetector");
	EagleTraffic *trafficDetector = new EagleTraffic(EAGLETRAFFIC_LIGHTTYPE_VERTICAL);
	EagleCrosswalk *crosswalkDetector = new EagleCrosswalk;
    
	int frameCnt = 0;
	AVCData avcData;
	Mat current_frame;
	Mat last_frame;
	while (1) 
	{
		frameCnt++;

		int key = waitKey ( 30 );
		g_pushedKey = key;

		if (key == 'q' || key == 'Q')
			break;
		
		if (key == 'p') 
			isPaused = !isPaused;
		
		if (isPaused)
			continue;
		
		if (key == 's') 
		{
			if (isPausedWithLastFrame == true)
				isPausedWithLastFrame = false;
			else 
				isPausedWithLastFrame = true;

		}

		// 프로세싱을 계속하는 일시정지
		if (!isPausedWithLastFrame)
		{
			cap >> current_frame;
			current_frame.copyTo(last_frame);
		}
		else
		{
			last_frame.copyTo(current_frame);
			//current_frame = last_frame;
		}
		
		if (!current_frame.empty() )
		{
			//imshow("Original", current_frame);
			
            
            // 버드아이뷰
            //getBirdEyeView(current_frame);
			//laneDetector->runModule(current_frame, cv::Rect(232,0,261,current_frame.rows));
            
            // 디텍팅 시작
            AVCTimeProfiler::begin();
			
            //laneDetector->runModule(current_frame, cv::Rect(0,310,current_frame.cols,current_frame.rows-310));
            
            //roadAreaDetector->runModule(current_frame, cv::Rect(0,0,1,1));
			

			int trafficSign = trafficDetector->decideTrafficLight(current_frame);
			int crosswalkPos = crosswalkDetector->decideCrosswalk(current_frame);

			// ROI 부분의 레퍼런스만 갖고가기때문에 실제 current_frame위에 사각형 그려진다.
			pedDetector->runModule(current_frame, cv::Rect(320,170,current_frame.cols-320,current_frame.rows-170-100));

			imshow("Original",current_frame);

			//cout << "current frame" << frameCnt << endl;
           AVCTimeProfiler::end();
            
		   if(pedDetector->isFound)
		   {
			   avcData.missonID = 10;
			   avcData.value1 = pedDetector->distance;
			   avcData.value2 = -1;
		   }
		   else if (trafficSign == EAGLETRAFFIC_SIGN_RED || trafficSign == EAGLETRAFFIC_SIGN_GREEN)
		   {
			   avcData.missonID = 20;
			   avcData.value1 = trafficSign ;
			   avcData.value2 = crosswalkPos; // MOD: 미터로 환산할것
		   }
		   else if (trafficSign == EAGLETRAFFIC_SIGN_LEFT || trafficSign == EAGLETRAFFIC_SIGN_RIGHT)
		   {
			   avcData.missonID = 30;
			   avcData.value1 = trafficSign ;
			   avcData.value2 = -1;
		   }
		   else
		   {
			   avcData.missonID = 0;
			   avcData.value1 = -1;
			   avcData.value2 = -1;
		   }


            if( isNetworkOn )
            {
				
				/*
				// 결과 데이터 패키징해서 전송
                avcData.marginLeft = 10;
                avcData.marginRight = 20;
                avcData.laneValidity = 5;   
                avcData.trafficSign = trafficSign;
				avcData.crosswalkPos = crosswalkPos;
//				avcData.angleLeft = laneDetector->angleLeft;
	//			avcData.angleRight = laneDetector->angleRight;
                avcData.trafficSign = 1;
				avcData.isPedDetected = pedDetector->isFound;
				avcData.pedDistance = pedDetector->distance;
                */
                
                netModule->addToQueue(avcData);
            }
            
		}
		else 
        {
            if( !isSourceLive )
            {
                // 영상 무한반복
				frameCnt = 0;
                cap.open(filePath);
            }
		}
        
	}
	
    
    
    // 리소스 해제
    if (netModule != NULL) 
    {
        netModule->stop();
        delete netModule;
    }
    
    //delete laneDetector;
	delete pedDetector;
	
}

//void getBirdEyeView(Mat& src, CV_OUT Mat& dst)
void getBirdEyeView(Mat& src)
{
	//멀리보기 모드
	int translation= 36;
	int dist = 377;
	
	// 가까이보기
	//	int translation_= 77;
	//	int dist_ = 528;
	
	
	// 회전각 셋팅, x축 회전각은 0도
	double alpha=0, beta=90., gamma=90.;
	alpha = (alpha - 90.)*CV_PI/180.0;
	beta = (beta - 90.)*CV_PI/180.0;			
	gamma = (gamma - 90.)*CV_PI/180.0;
	
	// 캘리브레이트 된 카메라 파라메터 셋팅
	int fx = 816,fy = 818;
	int cx = 338, cy = 260;
	Mat distortionMat = (Mat_<double>(1,5) << -0.34060,   -0.11217,   -0.00034,   0.00070,  0.00000 );
	
	// Projection 2D -> 3D matrix
	Mat A1 = (Mat_<double>(4,3) <<
			  1, 0, -cx,
			  0, 1, -cy,
			  0, 0,    0,
			  0, 0,    1);
	
	// Rotation matrices around the X,Y,Z axis
	Mat RX = (Mat_<double>(4, 4) <<
			  1,          0,           0, 0,
			  0, cos(alpha), -sin(alpha), 0,
			  0, sin(alpha),  cos(alpha), 0,
			  0,          0,           0, 1);
	
	Mat RY = (Mat_<double>(4, 4) <<
			  cos(beta), 0, -sin(beta), 0,
			  0, 1,          0, 0,
			  sin(beta), 0,  cos(beta), 0,
			  0, 0,          0, 1);
	
	Mat RZ = (Mat_<double>(4, 4) <<
			  cos(gamma), -sin(gamma), 0, 0,
			  sin(gamma),  cos(gamma), 0, 0,
			  0,          0,           1, 0,
			  0,          0,           0, 1);
	
	// Composed rotation matrix with (RX,RY,RZ)
	Mat R = RX * RY * RZ;
	
	// Translation matrix on the Z axis change dist will change the height
	Mat T = (Mat_<double>(4, 4) <<
			 1, 0, 0, 0,
			 0, 1, 0, translation,
			 0, 0, 1, dist,
			 0, 0, 0, 1);
	
	// Camera Intrisecs matrix 3D -> 2D
	Mat A2 = (Mat_<double>(3,4) <<
			  fx, 0, cx, 0,
			  0, fy, cy, 0,
			  0, 0,   1, 0);
	
	// Final and overall transformation matrix
	Mat transformMat = A2 * (T * (R * A1));
	
	// Camera Intrisecs matrix 3D -> 2D
	Mat intrinsicMat = (Mat_<double>(3,3) <<
						fx, 0, cx, 
						0, fy, cy, 
						0, 0,   1);
	
	
	
	Mat source_undistorted;
	undistort(src,source_undistorted,intrinsicMat,distortionMat);
	
	// 소스자체에 overwrite
	warpPerspective(source_undistorted, src, transformMat, source_undistorted.size(), INTER_CUBIC | WARP_INVERSE_MAP);
	
}



#ifdef MAC_OS_X_VERSION_10_7
string getFilePathFromBundle(const char* aName)
{
	char path[512];
	
	CFBundleRef mainBundle  = CFBundleGetMainBundle ();
    assert (mainBundle);
	
	CFStringRef cfStringRef; 
	cfStringRef = CFStringCreateWithCString(kCFAllocatorDefault, 
											aName, 
											kCFStringEncodingMacRoman); 
	
	
	CFURLRef video_url = CFBundleCopyResourceURL(mainBundle, cfStringRef, NULL, CFSTR("samples") );
	assert (video_url);
	
	bool isFileOpened = CFURLGetFileSystemRepresentation (video_url, true, reinterpret_cast<UInt8 *>(path), 512);
	if (!isFileOpened)
        assert(0);
	
	CFRelease(cfStringRef); 
    
	
	string pathStr = path;
	return pathStr;
}

#endif

