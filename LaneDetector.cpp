/*
 *  LaneDetector.cpp
 *  AVC
 *
 *  Created by YoungJae Kwon on 11. 8. 11..
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "LaneDetector.h"

using namespace cv;
using namespace std;

static int bin_threshold;
static int bin_thresholdType;
static int hough_threshold;
static int r_min,g_min,b_min,b_max;


//도로 차선폭 대략 2.5미터, 카메라로 430픽셀
//스포티지 1.9미터


int erosion_type = 0;
int erosion_size = 0;
int dilation_type = 0;
int dilation_size = 0;
int const max_elem = 2;
int const max_kernel_size = 21;


void onTrackBarChange_LaneDetector( int value, void* aBarType)
{
	LaneDetectorTrackBarType barType = *((LaneDetectorTrackBarType*)aBarType);
	switch (barType) 
	{
		case LaneDetectorTrackBarBinaryThreshold:
			bin_threshold = value;
			break;
		case LaneDetectorTrackBarBinaryThresholdType:
			bin_thresholdType = value;
			break;
		case LaneDetectorTrackBarHoughThreshold:
			hough_threshold = value;
			break;
		case LaneDetectorTrackBarRMin:
			r_min = value;
			break;
		case LaneDetectorTrackBarGMin:
			g_min = value;
			break;
		case LaneDetectorTrackBarBMax:
			b_max = value;
			break;
		case LaneDetectorTrackBarBMin:
			b_min = value;
			break;
			
		default:
			break;
	} 
}
void onMouse_LaneDetector( int event, int x, int y, int, void* )
{
    switch( event )
    {
        case CV_EVENT_LBUTTONDOWN:
            
            break;
        case CV_EVENT_LBUTTONUP:
            
            break;
    }
}

/*void Probabilistic_Hough( int, void* )
 {
 vector<Vec4i> p_lines;
 cvtColor( edges, probabilistic_hough, CV_GRAY2BGR );
 
 /// 2. Use Probabilistic Hough Transform
 HoughLinesP( edges, p_lines, 1, CV_PI/180, min_threshold + p_trackbar, 30, 10 );
 
 /// Show the result
 for( size_t i = 0; i < p_lines.size(); i++ )
 {
 Vec4i l = p_lines[i];
 line( probabilistic_hough, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255,0,0), 3, CV_AA);
 }
 
 imshow( probabilistic_name, probabilistic_hough );
 }*/

LaneDetector::LaneDetector(const char* aWindowName)
{
	windowName = aWindowName;
	winName_binary = "binary";
	winName_hough = "Hough TR";
	winName_trackbar = "Params";
	
	
	R_Yellow_LaneHistorySum = 0;
	L_Yellow_LaneHistorySum = 0;
	
	R_LaneHistorySum = 0;
	L_LaneHistorySum = 0;
	
    // 픽셀 -> 거리 변환계수
    // 404 cm : 640 px
	pixToDistRatio = 404.0/640.0;
	
	
	// 라인 튀는것 방지위한 평균갯수
	maxNumberOfHistory = 20;
	
	FileStorage fs("laneDetector.xml", FileStorage::READ);
	if( !fs.isOpened() )
	{
		bin_threshold = 128;
		bin_thresholdType = 1;
		hough_threshold = 50;
		
		// 노란색값 추출
		r_min = 100;
		g_min = 70;
		b_min = 30;
		b_max = 77;
	}
	else 
	{
		fs["bin_threshold"] >> bin_threshold;
		fs["bin_thresholdType"] >> bin_thresholdType;
		fs["hough_threshold"] >> hough_threshold;
		
		fs["r_min"] >>r_min;
		fs["g_min"] >> g_min;
		fs["b_min"] >> b_min;
		fs["b_max"] >> b_max;
	}
	
	
	
	// create all necessary GUI instances
    namedWindow (windowName, CV_WINDOW_AUTOSIZE);
    setMouseCallback( windowName, onMouse_LaneDetector, 0 );
	
	
	namedWindow (winName_trackbar, CV_WINDOW_AUTOSIZE);	
	//트랙바 이름, 갖다붙일 윈도우 이름, 조정될 값, 값의 맥시멈, 변동시 호출되는 콜백함수
	
	barTypeBT = LaneDetectorTrackBarBinaryThreshold;
	createTrackbar( "bin_threshold", winName_trackbar, &bin_threshold, 255, onTrackBarChange_LaneDetector, &barTypeBT );
	
	barTypeBTT = LaneDetectorTrackBarBinaryThresholdType;
	createTrackbar( "bin_thresholdType", winName_trackbar, &bin_thresholdType, 4, onTrackBarChange_LaneDetector, &barTypeBTT );
	
	
	barTypeRMin = LaneDetectorTrackBarRMin;
	createTrackbar( "r_min", winName_trackbar, &r_min, 255, onTrackBarChange_LaneDetector, &barTypeRMin );
	
	barTypeGMin = LaneDetectorTrackBarGMin;
	createTrackbar( "g_min", winName_trackbar, &g_min, 255, onTrackBarChange_LaneDetector, &barTypeGMin );
	
	barTypeBMin = LaneDetectorTrackBarBMin;
	createTrackbar( "b_min", winName_trackbar, &b_min, 255, onTrackBarChange_LaneDetector, &barTypeBMin );
	
	barTypeBMax = LaneDetectorTrackBarBMax;
	createTrackbar( "b_max", winName_trackbar, &b_max, 255, onTrackBarChange_LaneDetector, &barTypeBMax );
	
	
	/// Create Trackbars for Thresholds
	namedWindow( winName_hough, CV_WINDOW_AUTOSIZE );
	
	barTypeHough = LaneDetectorTrackBarHoughThreshold;
	createTrackbar( "hougha_tb", winName_hough, &hough_threshold, 150,onTrackBarChange_LaneDetector, &barTypeHough);
	
	barTypeErosionType = LaneDetectorTrackBarErosionType;
	createTrackbar( "erosion_type", winName_trackbar, &erosion_type, 3, onTrackBarChange_LaneDetector, &barTypeErosionType );
	
	barTypeErosionSize = LaneDetectorTrackBarErosionType;
	createTrackbar( "erosion_size", winName_trackbar, &erosion_size, 80, onTrackBarChange_LaneDetector, &barTypeErosionSize );
	
}
void LaneDetector::runModule(Mat &frame, cv::Rect roiRect)
{
	Mat dummy;
	cv::Mat ROI(frame, roiRect);
	
	imshow(windowName, frame);
	imshow(winName_trackbar, frame); // 더미영상없으면 트랙바가 안나와서 추가
	
	// 흰색/노란색 차선 색상인지
	// 스레숄딩 + 캐니엣지
	// 허프 트랜스폼
	// 각도 계산 및 거리 측정
	
	
	Mat g_ROI; // 추출된관심영역(small)의 그레이스케일 버전
	Mat g_yellowExtracted; // 관심영역에서 노란색 영역 추출
	Mat g_thresholded; // 관심영역에서 스레숄드 건 이미지 
	
	Mat frameToShow;
	cvtColor( ROI, g_ROI, CV_RGB2GRAY );
	
	cv::threshold( g_ROI, g_thresholded, bin_threshold, 255, bin_thresholdType );
	
	// RGB색상으로 뽑아오기
	g_yellowExtracted = Mat::zeros(ROI.rows,ROI.cols, CV_8UC1);
	vector<Mat> ch;
	split(ROI, ch);
	
	for( int y = 0; y < ROI.rows; y++ )
	{
		uchar* row_b = ch[0].ptr<uchar>(y);
		uchar* row_g = ch[1].ptr<uchar>(y);
		uchar* row_r = ch[2].ptr<uchar>(y);
		for( int x = 0; x < ROI.cols; x++ )
		{
			uchar& value = g_yellowExtracted.at<uchar>(y, x);
			//			if( 100 <= row_b[x] && row_b[x] <= 255 &&
			//			   100 <= row_g[x] && row_g[x] <= 255 &&
			//			   100 <= row_r[x] && row_r[x] <= 255 )
			//			{
			//				
			//				value = 123;
			//			}
			//			else {
			//				value = 0;
			//			}
			
			if( b_min <= (int)row_b[x] && (int)row_b[x] <= b_max &&
			   g_min <= (int)row_g[x] && (int)row_g[x] <= 255 &&
			   r_min <= (int)row_r[x] && (int)row_r[x] <= 255 )
			{
				
				value = 255;
			}	
			else 
			{
				value = 0;	
			}
		}
	}
	
	
	frameToShow = ROI; // 보여줄 이미지를 원본이미지로 설정 후 그 위에 라인을 드로잉
	//----------------------------------
	//1. 노란색관련 영역에서 차선 검출
	//----------------------------------
	cout << "\nYellow:\n";
	
	Mat g_erodedImage;
	erosion(g_yellowExtracted, g_erodedImage);
	imshow("g_yellowExtracted&eroded", g_erodedImage);
	
	detectFromBinaryImage(g_yellowExtracted, frameToShow, Scalar(0,192,255),  R_Yellow_LaneHistory, L_Yellow_LaneHistory, R_Yellow_LaneHistorySum, L_Yellow_LaneHistorySum);
	
	//----------------------------------
	//2. 스레숄딩된 흰색 영역에서 차선 검출
	//----------------------------------
	cout << "\ngeneral:\n";
	
	erosion(g_thresholded, g_erodedImage);
	imshow("g_thresholded&eroded", g_erodedImage);
	
	detectFromBinaryImage(g_thresholded, frameToShow,  Scalar(255,0,0), R_LaneHistory, L_LaneHistory, R_LaneHistorySum, L_LaneHistorySum);		
	cout << endl;
	
	imshow( winName_hough, frameToShow );
	
    
    return ;	
}

void LaneDetector::detectFromBinaryImage(Mat& g_targetImage, Mat& result, Scalar color,std::list<Vec2f>& rightLineHistory,std::list<Vec2f>& leftLineHistory, Vec2f& rightLineHistorySum, Vec2f& leftLineHistorySum )
{
	
	Mat edges;
	Canny( g_targetImage, edges, 50, 200, 3 );
	
	vector<Vec2f> s_lines;
	vector<Vec2f> left_lines;
	vector<Vec2f> right_lines;
	
	//cvtColor( edges, result, CV_GRAY2BGR );
	
	HoughLines( edges, s_lines, 1, CV_PI/180, hough_threshold, 0, 0 ); 
	
	for( int i = 0; i < s_lines.size(); i++ )
	{
		float r = s_lines[i][0], theta = s_lines[i][1];
		float degree = theta * 180 / CV_PI;
		
		// 좌측차선/우측차선 분리 저장
		if(0 < degree && degree < 45) // lane on rightside
		{
			right_lines.push_back(s_lines[i]);
		}
		else if(135 < degree && degree < 180) // lane on leftside
		{
			left_lines.push_back(s_lines[i]);
		}
		
		// birdeye 디텍터
//		if ( (0 < degree && degree < 15) || (165 < degree && degree < 180) ) 
//		{
//			if(r > 0) // lane on rightside
//			{
//				right_lines.push_back(s_lines[i]);
//			}
//			else if(r < 0) // lane on leftside
//			{
//				left_lines.push_back(s_lines[i]);
//			}
//			
//		}
		
	} 
	eliminateOutLiers(right_lines);
	eliminateOutLiers(left_lines);
	
	// 히스토리에 기반해서 갑작스런 차선 튐 방지 maxNumberOfHistory 파라메터를 적절히 조절할것
	Vec2f rightLine;
	if ( getAverageLine(right_lines,rightLine) == true)
	{
		// 지정갯수이상 히스토리가 쌓이면 더이상 데이터를 쌓지 않고 버린다.
		if (rightLineHistory.size() > maxNumberOfHistory) 
		{
			rightLineHistorySum -= rightLineHistory.front();
			rightLineHistory.pop_front();
			rightLineHistory.push_back(rightLine);
			rightLineHistorySum += rightLine;
		}
		else 
		{
			rightLineHistory.push_back(rightLine);
			rightLineHistorySum += rightLine;		
		}
	}
	else // 라인이 추출되지 않은 프레임일 경우 제일 오래된 히스토리 하나 삭제
	{
		
		if (rightLineHistory.size() > 0) 
		{
			rightLineHistorySum -= rightLineHistory.front();
			rightLineHistory.pop_front();
		}
	}
	
	
	Vec2f leftLine;
	if ( getAverageLine(left_lines,leftLine) == true)
	{
		if (leftLineHistory.size() > maxNumberOfHistory) 
		{
			leftLineHistorySum -= leftLineHistory.front();
			leftLineHistory.pop_front();
			leftLineHistory.push_back(leftLine);
			leftLineHistorySum += leftLine;
		}
		else 
		{
			leftLineHistory.push_back(leftLine);
			leftLineHistorySum += leftLine;
		}
	}
	else
	{	
		if (leftLineHistory.size() > 0) 
		{
			leftLineHistorySum -= leftLineHistory.front();
			leftLineHistory.pop_front();
		}
	}
	
    
	Vec2f rightToDraw = Vec2f( rightLineHistorySum[0]/rightLineHistory.size(),rightLineHistorySum[1]/rightLineHistory.size() );
	Vec2f leftToDraw = Vec2f( leftLineHistorySum[0]/leftLineHistory.size(),leftLineHistorySum[1]/leftLineHistory.size() ) ;
    
    // 허프 트랜스폼 좌표계 결정
    houghROI.height = edges.rows;
    houghROI.width = edges.cols;
    
	rightMargin = houghROI.width - drawDetectedLine( rightToDraw, result, color );
	leftMargin = drawDetectedLine( leftToDraw, result,color );

    
    
	// 결과값 저장
	angleLeft = leftToDraw[1] * 180 / CV_PI;
	angleRight = rightToDraw[1] * 180 / CV_PI; 
	
	// 로그값 보기 위함
	cout << "angle L:" << angleLeft << "\tR:" << rightToDraw[1] * 180 / CV_PI << endl; 
	
}

// 라인을 그리고 x절편을 리턴한다
int LaneDetector::drawDetectedLine(Vec2f& aLine, Mat& bgImage, Scalar color)
{
	float r = aLine[0], theta = aLine[1];
	double cos_t = cos(theta), sin_t = sin(theta);
	double x0 = r*cos_t, y0 = r*sin_t;
	double alpha = 1000;
	
    
	cv::Point pt1( cvRound(x0 + alpha*(-sin_t)), cvRound(y0 + alpha*cos_t) );
	cv::Point pt2( cvRound(x0 - alpha*(-sin_t)), cvRound(y0 - alpha*cos_t) );
	line( bgImage, pt1, pt2, color, 3, CV_AA);     


    // x절편구하기, 이미지좌표계이므로 y=0일때가 아니고 y = height 일때의 x값을 절편으로 정의
    float x_bottom = (r-houghROI.height*sin_t)/cos_t;
    cout << "x절편" << x_bottom << endl;    
    
    return (int)x_bottom;	
}

int LaneDetector::convertPixelToDistance(int aPixel)
{    
    return aPixel * pixToDistRatio;
}

// 아웃라이어들 제거
// 현재는 총개체 3개 이상일 경우 최대,최소만 제거
// MOD: 나중에 더 정밀한 필터나 툴을 사용할것
// ex) 이전프레임에서 피드백 받거나 칼만필터 등등
void LaneDetector::eliminateOutLiers(vector<Vec2f>& aLines)
{
	int num = aLines.size();
	if (num == 0 || num == 1 || num == 2) 
		return;
	else if (num >= 3)
	{
		// 평균을 내고 평균값에서 가장 거리가 먼 녀석을 제외
		Vec2f avg;
		getAverageLine(aLines,avg);
		float outLierValue = 0;
		float diff;
		int outLierIndex;
		
		// angle 아웃라이어 제거
		for (int i = 0; i < num; i++)
		{
			diff = abs(avg[1] - aLines[i][1]);
			if (diff > outLierValue )
			{
				outLierValue = diff;
				outLierIndex = i;
			}
		}
		aLines.erase(aLines.begin()+outLierIndex);
		
		// radius 아웃라이어 제거
		for (int i = 0; i < num; i++)
		{
			diff = abs(avg[0] - aLines[i][0]);
			if (diff > outLierValue )
			{
				outLierValue = diff;
				outLierIndex = i;
			}
		}
		aLines.erase(aLines.begin()+outLierIndex);
		return;
	}
}


bool LaneDetector::getAverageLine(vector<Vec2f>& aLines, Vec2f& avgLine)
{
	float avgAngle = 0;
	float avgRadius = 0;
	int num = aLines.size();
	
	// 라인이 없을경우 false 리턴
	if( num == 0 )
		return false;
	
	for (int i = 0; i < num; i++)
	{
		avgRadius += aLines[i][0];		
		avgAngle += aLines[i][1];
	}
	avgLine[0] = avgRadius / num;
	avgLine[1] = avgAngle / num;
	
	return true;
}

/**  @function Erosion  */
void LaneDetector::erosion( Mat &frame, Mat &result)
{	
	Mat element = getStructuringElement( erosion_type,
										cv::Size( 2*erosion_size + 1, 2*erosion_size+1 ),
										cv::Point( erosion_size, erosion_size ) );
	
	/// Apply the erosion operation
	
	// 침식/팽창/침식
	erode( frame, result, element );
	dilate( frame, result, element );
	erode( frame, result, element );
}

/** @function Dilation */
void LaneDetector::dilation( Mat &frame, Mat &result )
{	
	Mat element = getStructuringElement( dilation_type,
										cv::Size( 2*dilation_size + 1, 2*dilation_size+1 ),
										cv::Point( dilation_size, dilation_size ) );
	/// Apply the dilation operation
	dilate( frame, result, element );
	imshow( "Dilation Demo", result );
}



void LaneDetector::saveParamenters()
{
	FileStorage fs("laneDetector.xml", FileStorage::WRITE);
	
    fs << "bin_threshold" << bin_threshold;
	fs << "bin_thresholdType" << bin_thresholdType;
	fs << "hough_threshold" << hough_threshold;
	fs << "b_max" << b_max;
	fs << "r_min" << r_min;
	fs << "g_min" << g_min;
	fs << "b_min" << b_min;
	
	// 배열 저장 예제코드
	//    Mat cameraMatrix = (Mat_<double>(3,3) << 1000, 0, 320, 0, 1000, 240, 0, 0, 1);
	//    Mat distCoeffs = (Mat_<double>(5,1) << 0.1, 0.01, -0.001, 0, 0);
	//    fs << "cameraMatrix" << cameraMatrix << "distCoeffs" << distCoeffs;
	//    fs << "features" << "[";
	//    for( int i = 0; i < 3; i++ )
	//    {
	//        int x = rand() % 640;
	//        int y = rand() % 480;
	//        uchar lbp = rand() % 256;
	//		
	//        fs << "{:" << "x" << x << "y" << y << "lbp" << "[:";
	//        for( int j = 0; j < 8; j++ )
	//            fs << ((lbp >> j) & 1);
	//        fs << "]" << "}";
	//    }
	//    fs << "]";
    fs.release();
    return ;
	
}

LaneDetector::~LaneDetector()
{
	//	saveWindowParameters(windowName);
	//	saveWindowParameters(winName_hough);
	saveParamenters();
}