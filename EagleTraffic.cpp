#include "EagleTraffic.h"

EagleTraffic::EagleTraffic() {
	_trafficLightType = EAGLETRAFFIC_LIGHTTYPE_VERTICAL;
	red_previous_pos = -1;
	red_previous_value = 0;
	green_previous_pos = -1;
	green_previous_value = 0;
}

EagleTraffic::EagleTraffic(int type) {
	_trafficLightType = type;
	red_previous_pos = -1;
	red_previous_value = 0;
	green_previous_pos = -1;
	green_previous_value = 0;
}

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
double EagleTraffic::_angle( cv::Point pt1, cv::Point pt2, cv::Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
void EagleTraffic::_findSquares( const cv::Mat& image, vector<vector<cv::Point> >& squares )
{
	int thresh = 50, N = 2;
    squares.clear();
    
    vector<vector<cv::Point> > contours;
	cv::Mat gray;
	image.copyTo(gray);
    
    // find squares in every color plane of the image

    // find contours and store them all as a list
    findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

    vector<cv::Point> approx;
            
    // test each contour
    for( size_t i = 0; i < contours.size(); i++ )
    {
        // approximate contour with accuracy proportional
        // to the contour perimeter
        approxPolyDP(cv::Mat(contours[i]), approx, arcLength(cv::Mat(contours[i]), true)*0.02, true);
                
        // square contours should have 4 vertices after approximation
        // relatively large area (to filter out noisy contours)
        // and be convex.
        // Note: absolute value of an area is used because
        // area may be positive or negative - in accordance with the
        // contour orientation
        if( approx.size() == 4 &&
            fabs(contourArea(cv::Mat(approx))) > 100 &&
            isContourConvex(cv::Mat(approx)) )
        {
            double maxCosine = 0;

            for( int j = 2; j < 5; j++ )
            {
                // find the maximum cosine of the angle between joint edges
                double cosine = fabs(_angle(approx[j%4], approx[j-2], approx[j-1]));
                maxCosine = MAX(maxCosine, cosine);
            }

            // if cosines of all angles are small
            // (all angles are ~90 degree) then write quandrange
            // vertices to resultant sequence
            if( maxCosine < 0.3 )
                squares.push_back(approx);
        }
    }
}


// the function draws all the squares in the image
void EagleTraffic::_drawSquares( cv::Mat& image, const vector<vector<cv::Point> >& squares )
{
    for( size_t i = 0; i < squares.size(); i++ )
    {
        const cv::Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        polylines(image, &p, &n, 1, true, cv::Scalar(255,255,0), 3, CV_AA);
    }

    imshow("squares", image);
}

int EagleTraffic::decideTrafficLight(const cv::Mat &image, int crosswalk_value) {
	if (image.empty()) return EAGLETRAFFIC_SIGN_NONE;

	float radius;
	cv::Point2f center;
	cv::Mat H = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));

	float width, height;
	
	cv::Mat roi_image;
	if (_trafficLightType == EAGLETRAFFIC_LIGHTTYPE_VERTICAL) {
		roi_image = image(cvRect(((double)image.cols)/5.0*1.5, image.rows/5, ((double)image.cols)/5.0*3.5, image.rows/2));
	}
	else {
		roi_image = image(cvRect(((double)image.cols)/5.0*0.5, 0, ((double)image.cols)/5.0*4.0, image.rows/2));
	}
	roi_image.copyTo(current_frame);

	// Convert from BGR to HSV
	cv::Mat hsvimage;
	cvtColor(current_frame, hsvimage, CV_BGR2HSV);
	vector<cv::Mat> hsv;
	split(hsvimage, hsv);
	
	/*cv::Mat square_bin;
	inRange(hsv[2], 60, 256, square_bin);
	dilate(square_bin, square_bin, H);
	erode(square_bin, square_bin, H);
	bitwise_not(square_bin, square_bin);
	imshow("square_bin", square_bin);

    vector<vector<cv::Point> > squares;
    _findSquares(square_bin, squares);
    _drawSquares(roi_image, squares);*/
	
	//cv::equalizeHist(hsv[0], hsv[0]);
	//cv::equalizeHist(hsv[2], hsv[2]);
	//cv::equalizeHist(hsv[1], hsv[1]);
	
	/*
	cv::Mat back_lightness_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);

	inRange(hsv[2], 64, 256, back_lightness_bin);

	cv::Mat red_hue_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat red_lightness_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat red_saturation_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat red_combine = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);

	inRange(hsv[0], 35, 256, red_hue_bin);
	inRange(hsv[2], 0, 208, red_lightness_bin);
	inRange(hsv[1], 0, 96, red_saturation_bin);

	max(red_saturation_bin, red_lightness_bin, red_combine);
	//max(red_combine, red_saturation_bin, red_combine);

	cv::Mat H = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));
	
	dilate(red_combine, red_combine, H);
	dilate(red_combine, red_combine, H);
	erode(red_combine, red_combine, H);
	erode(red_combine, red_combine, H);
	
	imshow("hue", hsv[0]);
	imshow("lightness", hsv[2]);
	imshow("saturation", hsv[1]);
	
	imshow("back_lightness_bin", back_lightness_bin);
	
	imshow("red_hue_bin", red_hue_bin);
	imshow("red_lightness_bin", red_lightness_bin);

	imshow("red_saturation_bin", red_saturation_bin);
	imshow("red_combine", red_combine);
	*/
	
		
	// Find red, green signal
	cv::Mat hr_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat hg_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat hg1_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat hg2_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat sr1_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat sr2_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat sr_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat sg_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat vr_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat hsr_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat hsg_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat hsvr_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat hsvg_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	
	// Hue 추출
	inRange(hsv[0], 15, 165, hr_bin); // Red Hue
	inRange(hsv[0], 0, 55, hg1_bin); // Green Hue1
	inRange(hsv[0], 95, 256, hg2_bin); // Green Hue2
	max(hg1_bin, hg2_bin, hg_bin); // Green Hue(Final)
	dilate(hr_bin, hr_bin, H);
	dilate(hr_bin, hr_bin, H);
	erode(hr_bin, hr_bin, H);
	erode(hr_bin, hr_bin, H);
	erode(hr_bin, hr_bin, H);
	erode(hr_bin, hr_bin, H);
	dilate(hr_bin, hr_bin, H);
	dilate(hr_bin, hr_bin, H);

	erode(hg_bin, hg_bin, H);
	dilate(hg_bin, hg_bin, H);

	inRange(hsv[2], 0, 200, vr_bin); // Value
	erode(vr_bin, vr_bin, H);
	dilate(vr_bin, vr_bin, H);

	// Saturation 추출
	//inRange(hsv[1], 0, 15, sr1_bin);
	//inRange(hsv[1], 241, 256, sr2_bin);
	//max(sr1_bin, sr2_bin, sr_bin); // Red Saturation(Final)
	inRange(hsv[1], 0, 60, sr_bin);
	inRange(hsv[1], 0, 60, sg_bin); // Green Saturation

	max(hr_bin, sr_bin, hsr_bin);
	max(hsr_bin, vr_bin, hsvr_bin);
	max(hg_bin, sg_bin, hsg_bin);
	max(hsg_bin, vr_bin, hsvg_bin);

	dilate(hsvr_bin, hsvr_bin, H);
	dilate(hsvr_bin, hsvr_bin, H);
	erode(hsvr_bin, hsvr_bin, H);
	erode(hsvr_bin, hsvr_bin, H);
	erode(hsvr_bin, hsvr_bin, H);

	dilate(hsvg_bin, hsvg_bin, H);
	dilate(hsvg_bin, hsvg_bin, H);
	erode(hsvg_bin, hsvg_bin, H);
	erode(hsvg_bin, hsvg_bin, H);
	
	/*imshow("hr_bin", hr_bin);
	imshow("hg_bin", hg_bin);
	imshow("vr_bin", vr_bin);
	imshow("sr_bin", sr_bin);
	imshow("sg_bin", sg_bin);
	imshow("hsvr_bin", hsvr_bin);
	imshow("hsvg_bin", hsvg_bin);
	imshow("hue", hsv[0]);
	imshow("satuation", hsv[1]);
	imshow("value", hsv[2]);*/
	

	
	// Find contours
	vector< vector<cv::Point> > contoursr;
	vector< cv::Vec4i > hierarchyr;
	findContours(hsvr_bin, contoursr, hierarchyr, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	vector< vector<cv::Point> > contoursg;
	vector< cv::Vec4i > hierarchyg;
	findContours(hsvg_bin, contoursg, hierarchyg, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	
	//imshow("vr_bin", vr_bin);

	int max_red_pos = -1;
	int max_red_value = 0;
	int max_green_pos = -1;
	int max_green_value = 0;
	

	// Detect traffic light(red)
	int redpos = 0;

	if (_trafficLightType == EAGLETRAFFIC_LIGHTTYPE_VERTICAL) {
		for(int i=0; i<contoursr.size(); i++){
			if (contoursr[i].size() <= 6) continue;
			//minEnclosingCircle(cv::Mat(contoursr[i]), center, radius);
			cv::RotatedRect testRect = fitEllipse(cv::Mat(contoursr[i]));
			CvRect testRect2 = testRect.boundingRect();

			double ratio = ((double)testRect2.width / (double)testRect2.height);

			CvRect resultRect = CvRect(testRect2);
			resultRect.width = ((double)resultRect.width * 0.8);
			resultRect.height = ((double)resultRect.height * 0.8);
			resultRect.x = (double)resultRect.x + ((double)testRect2.width * 0.1);
			resultRect.y = (double)resultRect.y + ((double)testRect2.height * 0.1);
			if (ratio < 0.5 || ratio > 1.5) continue;
			if (resultRect.width <= 4 || resultRect.width >= 40) continue;
			if (resultRect.height <= 4 || resultRect.height >= 40) continue;

			double rectSize = (resultRect.width * resultRect.height);

			if (!(0 < resultRect.x-resultRect.width && resultRect.x+(resultRect.width*2) < vr_bin.cols)) continue;
			if (!(0 < resultRect.y-resultRect.height && resultRect.y+(resultRect.height*2) < vr_bin.rows)) continue;

			// 바이너리제이션 값 분석
			// 적색 신호등 분석
			//int center_bin_count = countNonZero(vr_bin(resultRect));
			//if (center_bin_count > rectSize * 0.5) continue;
		
			rectangle(current_frame, resultRect, cv::Scalar(255,255,0), 2, 8, 0);

			// 신호등 아래쪽 분석
			//int bottom_bin_count = countNonZero(vr_bin(cvRect(resultRect.x, resultRect.y+resultRect.height, resultRect.width, resultRect.height)));
			//if (bottom_bin_count < rectSize * 0.5) continue;

			int center_value_count = cv::sum(hsv[2](cvRect(resultRect.x, resultRect.y, resultRect.width, resultRect.height)))[0];

			// 명암 값 분석
			// 신호등 아래쪽의 명암 총 합 계산
			int bottom_value_count = cv::sum(hsv[2](cvRect(resultRect.x, resultRect.y+resultRect.height, resultRect.width, resultRect.height)))[0];
			if (bottom_value_count > center_value_count * 1.2) continue;

			// 신호등 왼쪽의 명암 총 합 계산
			int left_value_count = cv::sum(hsv[2](cvRect(resultRect.x-resultRect.width, resultRect.y, resultRect.width, resultRect.height)))[0];
			if (left_value_count < bottom_value_count * 1.5) continue;

			// 신호등 오른쪽의 명암 총 합 계산
			int right_value_count = cv::sum(hsv[2](cvRect(resultRect.x+resultRect.width, resultRect.y, resultRect.width, resultRect.height)))[0];
			if (right_value_count < bottom_value_count * 1.5) continue;

			// 신호등 위쪽의 명암 총 합 계산
			int top_value_count = cv::sum(hsv[2](cvRect(resultRect.x, resultRect.y-resultRect.height, resultRect.width, resultRect.height)))[0];
			if (top_value_count < bottom_value_count * 1.5) continue;

			// 신호등 주변의 Hue값 분포 분석
			//int near_hue_count = (rectSize*9)-countNonZero(hr_bin(cvRect(resultRect.x-resultRect.width, resultRect.y-resultRect.height, resultRect.width*3, resultRect.height*2)));
			//int center_hue_count = rectSize-countNonZero(hr_bin(resultRect));
			//if ((near_hue_count-center_hue_count) > center_hue_count*2) continue;
			//cout << near_hue_count << " " << center_hue_count << endl;
			
			rectangle(current_frame, cvRect(resultRect.x-1, resultRect.y-1, resultRect.width+2, resultRect.height*2+2), cv::Scalar(0,255,255), 2, 8, 0);
			rectangle(current_frame, resultRect, cv::Scalar(0,0,255), 2, 8, 0);

			int current_pos = resultRect.y+resultRect.height/2;

			// 이전 결과 반영
			if (red_previous_pos == -1 || ((current_pos - red_previous_pos) > -4 && (current_pos - red_previous_pos) < 10)) {
				redpos = red_previous_value;
			} else {
				redpos = 0;
			}

			redpos += rectSize;

			// 횡단보도 가중치 부여
			if (crosswalk_value >= CROSSWALK_THRESHOLD) {
				redpos += ((crosswalk_value-CROSSWALK_THRESHOLD)*2);
			}

			if (redpos > max_red_value) {
				max_red_value = redpos;
				max_red_pos = current_pos;
			}
		}
	}

	// Detect traffic light(green)
	int greenpos = 0;
	for(int i=0; i<contoursg.size(); i++){
		if (contoursg[i].size() <= 6) continue;
		//minEnclosingCircle(cv::Mat(contoursg[i]), center, radius);
		cv::RotatedRect testRect = fitEllipse(cv::Mat(contoursg[i]));
		CvRect testRect2 = testRect.boundingRect();

		double ratio = ((double)testRect2.width / (double)testRect2.height);

		CvRect resultRect = CvRect(testRect2);
		resultRect.width = ((double)resultRect.width * 0.8);
		resultRect.height = ((double)resultRect.height * 0.8);
		resultRect.x = (double)resultRect.x + ((double)testRect2.width * 0.1);
		resultRect.y = (double)resultRect.y + ((double)testRect2.height * 0.1);
		if (ratio < 0.7 || ratio > 1.3) continue;
		if (resultRect.width <= 3 || resultRect.width >= 40) continue;
		if (resultRect.height <= 3 || resultRect.height >= 40) continue;

		double rectSize = (resultRect.width * resultRect.height);

		if (!(0 < resultRect.x-resultRect.width && resultRect.x+(resultRect.width*2) < vr_bin.cols)) continue;
		if (!(0 < resultRect.y-resultRect.height && resultRect.y+(resultRect.height*2) < vr_bin.rows)) continue;

		// 바이너리제이션 값 분석
		// 녹색 신호등 분석
		//int center_bin_count = countNonZero(vr_bin(resultRect));
		//if (center_bin_count > rectSize * 0.5) continue;
		
		rectangle(roi_image, resultRect, cv::Scalar(255,255,0), 2, 8, 0);

		if (_trafficLightType == EAGLETRAFFIC_LIGHTTYPE_VERTICAL) {

			// 신호등 위쪽 분석
			//int top_bin_count = countNonZero(vr_bin(cvRect(resultRect.x, resultRect.y-resultRect.height, resultRect.width, resultRect.height)));
			//if (top_bin_count < rectSize * 0.5) continue;

			// 명암 값 분석
			// 신호등 위쪽의 명암 총 합 계산
			int top_value_count = cv::sum(hsv[2](cvRect(resultRect.x, resultRect.y-resultRect.height, resultRect.width, resultRect.height)))[0];
			if (top_value_count > rectSize * 96) continue;

			// 신호등 왼쪽의 명암 총 합 계산
			int left_value_count = cv::sum(hsv[2](cvRect(resultRect.x-resultRect.width, resultRect.y, resultRect.width, resultRect.height)))[0];
			if (left_value_count < top_value_count * 1.2) continue;

			// 신호등 오른쪽의 명암 총 합 계산
			int right_value_count = cv::sum(hsv[2](cvRect(resultRect.x+resultRect.width, resultRect.y, resultRect.width, resultRect.height)))[0];
			if (right_value_count < top_value_count * 1.2) continue;

			// 신호등 아래쪽의 명암 총 합 계산
			int bottom_value_count = cv::sum(hsv[2](cvRect(resultRect.x, resultRect.y+resultRect.height, resultRect.width, resultRect.height)))[0];
			if (bottom_value_count < top_value_count * 1.2) continue;
			
			rectangle(roi_image, cvRect(resultRect.x-1, resultRect.y-resultRect.height-1, resultRect.width+2, resultRect.height*2+2), cv::Scalar(0,255,255), 2, 8, 0);
			rectangle(roi_image, resultRect, cv::Scalar(0,255,0), 2, 8, 0);
			
			int current_pos = resultRect.y+resultRect.height/2;

			// 이전 결과 반영
			if (green_previous_pos == -1 || ((current_pos - green_previous_pos) > -4 && (current_pos - green_previous_pos) < 10)) {
				greenpos = green_previous_value;
			} else {
				greenpos = 0;
			}

			greenpos += rectSize;

			// 횡단보도 가중치 부여
			if (crosswalk_value >= CROSSWALK_THRESHOLD) {
				greenpos += (crosswalk_value-CROSSWALK_THRESHOLD)*2;
			}

			if (greenpos > max_green_value) {
				max_green_value = greenpos;
				max_green_pos = current_pos;
			}
		}
		else {
			rectangle(current_frame, resultRect, cv::Scalar(0,255,0), 2, 8, 0);

			int direction = 0;
			// 신호등 왼쪽 분석
			int left_value_count = cv::sum(hsv[2](cvRect(resultRect.x-resultRect.width, resultRect.y, resultRect.width, resultRect.height)))[0];
			if (left_value_count <= rectSize * 128) direction += ((rectSize * 256) - left_value_count);
			// 신호등 오른쪽 분석
			int right_value_count = cv::sum(hsv[2](cvRect(resultRect.x+resultRect.width, resultRect.y, resultRect.width, resultRect.height)))[0];
			if (right_value_count <= rectSize * 128) direction -= ((rectSize * 256) - right_value_count);

			if (direction >= (rectSize * 32) && direction <= (rectSize * 32)) continue;

			greenpos += direction;

			if (direction < 0) { // 왼쪽 지시등
			}
			else { // 오른쪽 지시등
			}
		}
	}
	
	//imshow("traffic_frame", current_frame);


	if (_trafficLightType == EAGLETRAFFIC_LIGHTTYPE_VERTICAL) {
		// 계산 결과 반영
		if (max_red_pos != -1) {
			red_previous_pos = max_red_pos;
			red_previous_value = max_red_value;
		} else {
			red_previous_value -= (red_previous_value*2/5 + 10);
		}

		if (red_previous_value <= 0) {
			red_previous_pos = -1;
			red_previous_value = 0;
		}

		// 계산 결과 반영
		if (max_green_pos != -1) {
			green_previous_pos = max_green_pos;
			green_previous_value = max_green_value;
		} else {
			green_previous_value -= (green_previous_value*2/5 + 10);
		}

		if (green_previous_value <= 0) {
			green_previous_pos = -1;
			green_previous_value = 0;
		}

		//cout << red_previous_value << " " << green_previous_value << endl;

		if (green_previous_value > red_previous_value && green_previous_value >= 200) return EAGLETRAFFIC_SIGN_GREEN; //cout << "GREEN SIGN" << endl;
		else if (red_previous_value >= 200) return EAGLETRAFFIC_SIGN_RED; //cout << "RED SIGN" << endl;
		else return EAGLETRAFFIC_SIGN_NONE; //cout << "X" << endl;
	}
	else {
		if (greenpos <= -50) return EAGLETRAFFIC_SIGN_LEFT;
		else if (greenpos >= 50) return EAGLETRAFFIC_SIGN_RIGHT;
		else return EAGLETRAFFIC_SIGN_NONE;
	}
	
	
	return EAGLETRAFFIC_SIGN_NONE;
}