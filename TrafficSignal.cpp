
#include "opencv2/opencv.hpp"
#include <cassert>
#include <iostream>


const char  * WINDOW_NAME  = "Face Tracker";

const int CASCADE_NAME_LEN = 2048;
char    CASCADE_NAME[CASCADE_NAME_LEN];

const int VIDEO_NAME_LEN = 2048;
char VIDEO_NAME[VIDEO_NAME_LEN];

using namespace cv;
using namespace std;

int vmin = 80, vmax = 256, smin = 34;
const int scale = 4;



Size2i screenSize;

bool backprojMode = false;

bool isNewHistogramRequired = false;
bool showHist = true;


Point2i origin;
cv::Rect lockedRect;
bool isLocked = false;


// 찾아진 얼굴들 저장
vector<cv::Rect> faces;

cv::Rect ScaleRectOnCenter(cv::Rect srcRect, float scale)
{
    
    return cv::Rect(srcRect.x+srcRect.width*(1.0 - scale)/2 ,
                    srcRect.y+srcRect.height*(1.0 - scale)/2 ,
                    srcRect.width*scale,
                    srcRect.height*scale);
}
cv::Rect ScaleRectOnLeftTop(cv::Rect srcRect, float scale)
{
    
    return cv::Rect(srcRect.x*scale ,
                    srcRect.y*scale ,
                    srcRect.width*scale,
                    srcRect.height*scale);
}
// 각 축(x,y)별로 (a의 max > b의 min) && (a의 min < b의 max) 일때 두 rect가 overlap
bool isRectOverlap(cv::Rect a, cv::Rect b)
{
    if( (a.x+a.width > b.x) && (a.x < b.x + b.width) &&
       (a.y+a.height > b.y) && (a.y < b.y + b.height) )
        return true;
    else
        return false;
}



void getHSVAnalysis(Mat &current_frame, Mat &hsv)
{
	cvtColor(current_frame, hsv, CV_BGR2HSV);
	
	vector<Mat> planes;
	split(hsv, planes);
	
	for( int y = 0; y < hsv.rows; y++ )
	{
		uchar* row_h = planes[0].ptr<uchar>(y);
		uchar* row_s = planes[1].ptr<uchar>(y);
		uchar* row_v = planes[2].ptr<uchar>(y);
		for( int x = 0; x < hsv.cols; x++ )
		{
			
			// HSV 황색 검출
			if ( 15< row_h[x] && row_h[x] < 25 )
			{
				//row_h[x] = 45; // 황색상 H값 45로 통일
				row_s[x] = 255;
				row_v[x] = 255; // 명도 최대치
			}
			else {
				row_s[x] = 0;
				row_v[x] = 0; // 명도 최소
			}
			// 인덱싱 예제코드
			//uchar& Vxy = planes[2].at<uchar>(y, x);
			//Vxy = saturate_cast<uchar>((Vxy-128)/2 + 128);
		}
	}
	
	merge(planes,hsv);
	
	cvtColor(hsv, current_frame, CV_HSV2BGR);
	
}
int main (int argc, char * const argv[]) 
{
    // 카메라의 파라메터, 카메라 종류가 바뀌면 새로 셋팅 필요
    const int std_distance = 84.5; // 카메라-얼굴 표준거리 [cm]
    const int std_radius = 120; // 표준거리에서의 얼굴 반지름 [pixel]
    
    // 0 ~ 180사이의 hue값을 레벨당 16만큼 양자화
    int histQtzSize = 16;
  
    // hue varies from 0 to 179, see cvtColor
    float hueRanges[] = {0,180};
    const float* pHueRanges = hueRanges;
    Mat hsv, hue, mask, hist, histimg;// = Mat::zeros(200, 320, CV_8UC3);
    Mat backproj;
    
	Boolean isFileOpened;
    CFBundleRef mainBundle  = CFBundleGetMainBundle ();
    assert (mainBundle);
	
	CFURLRef video_url = CFBundleCopyResourceURL(mainBundle, CFSTR("test"), CFSTR("mp4"), NULL);
	assert (video_url);
	
	isFileOpened = CFURLGetFileSystemRepresentation (video_url, true, reinterpret_cast<UInt8 *>(VIDEO_NAME), VIDEO_NAME_LEN);
	if (!isFileOpened)
        abort ();
	
    VideoCapture cap;
   // cap.open(0);
	cap.open(VIDEO_NAME);
    if( !cap.isOpened() )
    {
        cout << "***Could not initialize capturing...***\n";
        return 0;
    }
  

    // locate haar cascade from inside application bundle
    // (this is the mac way to package application resources)
	
    CFURLRef cascade_url = CFBundleCopyResourceURL (mainBundle, CFSTR("haarcascade_frontalface_alt2"), CFSTR("xml"), NULL);
    assert (cascade_url);
    isFileOpened = CFURLGetFileSystemRepresentation (cascade_url, true, reinterpret_cast<UInt8 *>(CASCADE_NAME), CASCADE_NAME_LEN);
    if (!isFileOpened)
        abort ();
	
	
	CascadeClassifier cascade(CASCADE_NAME);
    if (cascade.empty())
        abort ();
	
	
	
    
    // create all necessary GUI instances
    namedWindow (WINDOW_NAME, CV_WINDOW_AUTOSIZE);
    setMouseCallback( WINDOW_NAME, onMouse, 0 );
    
    createTrackbar( "Vmin", WINDOW_NAME, &vmin, 256, 0 );
    createTrackbar( "Vmax", WINDOW_NAME, &vmax, 256, 0 );
    createTrackbar( "Smin", WINDOW_NAME, &smin, 256, 0 );
    

    Mat current_frame;
    cap >> current_frame;

    // 비디오 카메라 화면사이즈 측정
    screenSize = Size2i(current_frame.cols,current_frame.rows);

    cv::Point screenCenter = cv::Point(screenSize.width/2,screenSize.height/2);
    
    Mat draw_image = Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC3);
    Mat gray_image = Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
    Mat small_image = Mat::zeros(current_frame.rows/scale, current_frame.cols/scale, CV_8UC1);
    
    // http://www.cognotics.com/opencv/servo_2007_series/part_2/page_2.html 참조할것
    
    //while ((current_frame = cvQueryFrame (camera)))
    while(1)
    {
		cap >> current_frame;
        if( current_frame.empty() )
            break;
		
		
		Mat edges;
		cvtColor(current_frame, edges, CV_BGR2GRAY);
        GaussianBlur(edges, edges, cv::Size(7,7), 1.5, 1.5);
        Canny(edges, edges, 0, 100, 3);
        cv::imshow("edges", edges);
		

	
		// RGB색상으로 뽑아오기
		// Feature로 하는게 훨 나을듯
		Mat binaryImage = Mat::zeros(current_frame.rows,current_frame.cols, CV_8UC1);
		vector<Mat> ch;
		split(current_frame, ch);
		
		for( int y = 0; y < current_frame.rows; y++ )
		{
			uchar* row_b = ch[0].ptr<uchar>(y);
			uchar* row_g = ch[1].ptr<uchar>(y);
			uchar* row_r = ch[2].ptr<uchar>(y);
			for( int x = 0; x < current_frame.cols; x++ )
			{
				uchar& value = binaryImage.at<uchar>(y, x);
				if( 100 <= row_b[x] && row_b[x] <= 255 &&
					100 <= row_g[x] && row_g[x] <= 255 &&
				    100 <= row_r[x] && row_r[x] <= 255 )
				{

					value = 255;
				}
				else if( 30 <= row_b[x] && row_b[x] <= 60 &&
				   70 <= row_g[x] && row_g[x] <= 255 &&
				   100 <= row_r[x] && row_r[x] <= 255 )
				{

					value = 128;
				}	
				else {
					value = 0;

				}

					
			}
		}

		
		imshow(WINDOW_NAME, binaryImage);

		// 30ms기다렸다가 진행 기다리는동안 키입력 받을 수 있음
		int key = waitKey (30);
		if (key == 'q' || key == 'Q')
			break;
		    }
    
    return 0;
}
