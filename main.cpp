/*
 *  main.cpp
 *  AVC
 *
 *  Created by YoungJae Kwon on 11. 8. 11..
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "opencv2/opencv.hpp"
#include <cassert>
#include <iostream>
#include "LaneDetector.h"

#include "AVCNetwork.h"
#include "boost/thread.hpp"
#include "AVCTimeProfiler.h"
#include "RoadAreaDetector.h"
#include "PedDetector.h"


//#include <ctype.h>

using namespace cv;
using namespace std;

Mat image;

bool backprojMode = false;
bool selectObject = false;
int trackObject = 0;
bool showHist = true;

// MacType.h의 Rect,Point와 OpenCV와 충돌일어남 명시적으로 적어줄것
Point2i origin;
cv::Rect selection;

int vmin = 10, vmax = 256, smin = 30;

void onMouse2( int event, int x, int y, int, void* )
{
    
    if( selectObject )
    {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);
        
        selection &= cv::Rect(0, 0, image.cols, image.rows);
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

#ifdef MAC_OS
string getFilePathFromBundle(const char* aName);
#endif


void getBirdEyeView(Mat& src);

int main (int argc, char * const argv[]) 
{
    // 캠시프트 코드 시작
    cv::Rect trackWindow;
    RotatedRect trackBox;
    
    // 0 ~ 180사이의 hue값을 레벨당 16만큼 양자화
    int histQtzSize = 16;
    
    // hue varies from 0 to 179, see cvtColor
    float hueRanges[] = {0,180};
    const float* pHueRanges = hueRanges;

    namedWindow( "Histogram", CV_WINDOW_AUTOSIZE );
    namedWindow( "CamShift Demo", CV_WINDOW_AUTOSIZE );
    setMouseCallback( "CamShift Demo", onMouse2, 0 );
    //  createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
    //  createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
    //  createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );
    
    Mat hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
    
    // 캠시프트 코드 끝
    
    
	bool isPaused = false;
	bool isNetworkOn = true;
    bool isSourceLive = true;
	//char filePath[] = "curve_test.avi";
    char filePath[] = "ped1.avi";
    
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
	
	LaneDetector *laneDetector = new LaneDetector("LaneDetector");
    RoadAreaDetector *roadAreaDetector = new RoadAreaDetector("RoadAreaDetector");
    PedDetector *pedDetector = new PedDetector("PedDetector");
    
	AVCData avcData;
	Mat current_frame;
	while (1) 
	{
		int key = waitKey ( 30 );
		if (key == 'q' || key == 'Q')
		{
			break;
		}
		if (key == 'p') 
		{
			if (isPaused == true)
				isPaused = false;
			else 
				isPaused = true;
			
		}
		if (isPaused)
			continue;
		
		
		
		cap >> current_frame;
		if (!current_frame.empty() )
		{
			//imshow("Original", current_frame);
			current_frame.copyTo(image);
            
            //캠시프트 시작
            cvtColor(current_frame, hsv, CV_BGR2HSV);
            
            if( trackObject )
            {
                int _vmin = vmin, _vmax = vmax;
                
                inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
                        Scalar(180, 256, MAX(_vmin, _vmax)), mask);
                int ch[] = {0, 0};
                hue.create(hsv.size(), hsv.depth());
                mixChannels(&hsv, 1, &hue, 1, ch, 1);
                
                // 새로운 영역이 마우스에 의해서 선택되었을 경우 히스토그램 새로 추출
                if( trackObject < 0 )
                {
                    // hue를 가진 매트릭스에서 select된부분만을 새로 뽑아냄(Region Of Interest)
                    Mat roi(hue, selection), maskroi(mask, selection);
                    calcHist(&roi, 1, 0, maskroi, hist, 1, &histQtzSize, &pHueRanges);
					normalize(hist, hist, 0, 255, CV_MINMAX);
                    
                    trackWindow = selection;
                    trackObject = 1;
                    
                    histimg = Scalar::all(0);
                    int binW = histimg.cols / histQtzSize;
                    Mat buf(1, histQtzSize, CV_8UC3);
                    
                    for( int i = 0; i < histQtzSize; i++ )
                        buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./histQtzSize), 255, 255);
                    
                    cvtColor(buf, buf, CV_HSV2BGR);
                    
                    // 히스토그램을 계산해서 출력
                    for( int i = 0; i < histQtzSize; i++ )
                    {
                        int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
                        rectangle( histimg, 
                                  Point2i(i*binW, histimg.rows),
                                  Point2i((i+1)*binW, histimg.rows - val),
                                  Scalar(buf.at<Vec3b>(i)), -1, 8 );
                    }
                }
                
                // hue 이미지를 히스토그램 backprojection하여 face probability 이미지 생성(변수명 backproj)
                // Similarly to calcHist , at each location (x, y) the function collects the values from the selected 
                // channels in the input images and finds the corresponding histogram bin. 
                // But instead of incrementing it, the function reads the bin value, scales it by scale and stores in backProject(x,y)
                
                calcBackProject(&hue, 1, 0, hist, backproj, &pHueRanges);
                backproj &= mask;
                
                // CAM(Continuously Adaptive Mean) shift 
                // It finds this new location by starting at the previous location and computing the center of gravity of the face-probability values within a rectangle.
                // It then shifts the rectangle so it's right over the center of gravity
                
                // CamShift( 얼굴확률 이미지, 트랙킹할 렉트, 조건 )
                RotatedRect trackBox = CamShift(backproj, trackWindow,
                                                TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
                
                // backproject된  face probability를 이미지 형태로 출력
                if( backprojMode )
                    cvtColor( backproj, image, CV_GRAY2BGR );
                
                // 오브젝트 트래킹 타원 그리기
                ellipse( image, trackBox, Scalar(0,0,255), 3, CV_AA );
            }
            
            // 마우스 선택영역 반전
            if( selectObject && selection.width > 0 && selection.height > 0 )
            {
                Mat roi(image, selection);
                bitwise_not(roi, roi);
            }
            
            imshow( "CamShift Demo", image );
            imshow( "Histogram", histimg );
            
            switch(key)
            {
                case 'b':
                    backprojMode = !backprojMode;
                    break;
                case 'c':
                    trackObject = 0;
                    histimg = Scalar::all(0);
                    break;
                case 'h':
                    showHist = !showHist;
                    if( !showHist )
                        destroyWindow( "Histogram" );
                    else
                        namedWindow( "Histogram", 1 );
                    break;
                default:
                    ;
            }

            
            // 캠시프트 끝
            
            // 버드아이뷰
            //getBirdEyeView(current_frame);
			//laneDetector->runModule(current_frame, cv::Rect(232,0,261,current_frame.rows));
            
            // 일반
            AVCTimeProfiler::begin();
			
            //laneDetector->runModule(current_frame, cv::Rect(0,310,current_frame.cols,current_frame.rows-310));
            
            //roadAreaDetector->runModule(current_frame, cv::Rect(0,0,1,1));
            //pedDetector->runModule(current_frame, cv::Rect(320,170,current_frame.cols-320,current_frame.rows-170-100));            
            AVCTimeProfiler::end();
            AVCTimeProfiler::print();
            
            if( isNetworkOn )
            {
				// 결과 데이터 패키징해서 전송
                avcData.marginLeft = 10;
                avcData.marginRight = 20;
                avcData.laneValidity =5;   
                
                avcData.steering = 0.5;
				avcData.angleLeft = laneDetector->angleLeft;
				avcData.angleRight = laneDetector->angleRight;
                avcData.trafficSign = 1;
                
                
                netModule->addToQueue(avcData);
            }
            
		}
		else 
        {
            if( !isSourceLive )
            {
                // 영상 무한반복
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
    
    delete laneDetector;
	
}

void getHist(Mat& src)
{
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



#ifdef MAC_OS
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

