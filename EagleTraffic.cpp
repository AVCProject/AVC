#include "EagleTraffic.h"

EagleTraffic::EagleTraffic() {
	_trafficLightType = EAGLETRAFFIC_LIGHTTYPE_VERTICAL;
}

EagleTraffic::EagleTraffic(int type) {
	_trafficLightType = type;
}

int EagleTraffic::decideTrafficLight(const cv::Mat &image) {
	if (image.empty()) return EAGLETRAFFIC_SIGN_NONE;

	float radius;
	cv::Point2f center;

	float width, height;
	
	if (_trafficLightType == EAGLETRAFFIC_LIGHTTYPE_VERTICAL) {
		current_frame = image(cvRect(((double)image.cols)/5.0*1.5, 0, ((double)image.cols)/5.0*3.5, image.rows/2));
	}
	else {
		current_frame = image(cvRect(((double)image.cols)/5.0*0.5, 0, ((double)image.cols)/5.0*4.0, image.rows/2));
	}

	// Convert from BGR to HSV
	cv::Mat hsvimage;
	cvtColor(current_frame, hsvimage, CV_BGR2HSV);
	vector<cv::Mat> hsv;
	split(hsvimage, hsv);
	
	//cv::equalizeHist(hsv[0], hsv[0]);
	cv::equalizeHist(hsv[2], hsv[2]);
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

	inRange(hsv[2], 0, 200, vr_bin); // Value

	// Saturation 추출
	inRange(hsv[1], 0, 15, sr1_bin);
	inRange(hsv[1], 241, 256, sr2_bin);
	max(sr1_bin, sr2_bin, sr_bin); // Red Saturation(Final)
	inRange(hsv[1], 0, 60, sg_bin); // Green Saturation

	max(hr_bin, sr_bin, hsr_bin);
	max(hsr_bin, vr_bin, hsvr_bin);
	max(hg_bin, sg_bin, hsg_bin);
	max(hsg_bin, vr_bin, hsvg_bin);

	cv::Mat H = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));
	dilate(hsvr_bin, hsvr_bin, H);
	dilate(hsvr_bin, hsvr_bin, H);
	erode(hsvr_bin, hsvr_bin, H);
	erode(hsvr_bin, hsvr_bin, H);
	dilate(hsvg_bin, hsvg_bin, H);
	dilate(hsvg_bin, hsvg_bin, H);
	erode(hsvg_bin, hsvg_bin, H);
	erode(hsvg_bin, hsvg_bin, H);
	
	//imshow("hr_bin", hr_bin);
	imshow("hg_bin", hg_bin);
	imshow("vr_bin", vr_bin);
	//imshow("sr_bin", sr_bin);
	imshow("sg_bin", sg_bin);
	//imshow("hsvr_bin", hsvr_bin);
	imshow("hsvg_bin", hsvg_bin);
	imshow("hue", hsv[0]);
	imshow("satuation", hsv[1]);
	imshow("value", hsv[2]);
	

	
	// Find contours
	vector< vector<cv::Point> > contoursr;
	vector< cv::Vec4i > hierarchyr;
	findContours(hsvr_bin, contoursr, hierarchyr, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	vector< vector<cv::Point> > contoursg;
	vector< cv::Vec4i > hierarchyg;
	findContours(hsvg_bin, contoursg, hierarchyg, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	
	//imshow("vr_bin", vr_bin);
	

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
			if (ratio < 0.7 || ratio > 1.3) continue;
			if (resultRect.width <= 4 || resultRect.width >= 40) continue;
			if (resultRect.height <= 4 || resultRect.height >= 40) continue;

			double rectSize = (resultRect.width * resultRect.height);

			if (!(0 < resultRect.x-resultRect.width && resultRect.x+(resultRect.width*2) < vr_bin.cols)) continue;
			if (!(0 < resultRect.y-resultRect.height && resultRect.y+(resultRect.height*2) < vr_bin.rows)) continue;

			// 바이너리제이션 값 분석
			// 적색 신호등 분석
			int center_bin_count = countNonZero(vr_bin(resultRect));
			if (center_bin_count > rectSize * 0.5) continue;

			// 신호등 아래쪽 분석
			int bottom_bin_count = countNonZero(vr_bin(cvRect(resultRect.x, resultRect.y+resultRect.height, resultRect.width, resultRect.height)));
			if (bottom_bin_count < rectSize * 0.5) continue;

			// 명암 값 분석
			// 신호등 아래쪽의 명암 총 합 계산
			int bottom_value_count = cv::sum(hsv[2](cvRect(resultRect.x, resultRect.y+resultRect.height, resultRect.width, resultRect.height)))[0];
			if (bottom_value_count > rectSize * 128) continue;

			// 신호등 왼쪽의 명암 총 합 계산
			int left_value_count = cv::sum(hsv[2](cvRect(resultRect.x-resultRect.width, resultRect.y, resultRect.width, resultRect.height)))[0];
			if (left_value_count < bottom_value_count * 1.5) continue;

			// 신호등 오른쪽의 명암 총 합 계산
			int right_value_count = cv::sum(hsv[2](cvRect(resultRect.x+resultRect.width, resultRect.y, resultRect.width, resultRect.height)))[0];
			if (right_value_count < bottom_value_count * 1.5) continue;

			// 신호등 위쪽의 명암 총 합 계산
			int top_value_count = cv::sum(hsv[2](cvRect(resultRect.x, resultRect.y-resultRect.height, resultRect.width, resultRect.height)))[0];
			if (top_value_count < bottom_value_count * 1.5) continue;

			//rectangle(current_frame, resultRect, cv::Scalar(0,0,255), 2, 8, 0);

			redpos += rectSize;
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
			//rectangle(current_frame, resultRect, cv::Scalar(0,255,0), 2, 8, 0);
		if (ratio < 0.7 || ratio > 1.3) continue;
		if (resultRect.width <= 3 || resultRect.width >= 40) continue;
		if (resultRect.height <= 3 || resultRect.height >= 40) continue;

		double rectSize = (resultRect.width * resultRect.height);

		if (!(0 < resultRect.x-resultRect.width && resultRect.x+(resultRect.width*2) < vr_bin.cols)) continue;
		if (!(0 < resultRect.y-resultRect.height && resultRect.y+(resultRect.height*2) < vr_bin.rows)) continue;

		// 바이너리제이션 값 분석
		// 녹색 신호등 분석
		int center_bin_count = countNonZero(vr_bin(resultRect));
		if (center_bin_count > rectSize * 0.5) continue;

		if (_trafficLightType == EAGLETRAFFIC_LIGHTTYPE_VERTICAL) {

			// 신호등 위쪽 분석
			int top_bin_count = countNonZero(vr_bin(cvRect(resultRect.x, resultRect.y-resultRect.height, resultRect.width, resultRect.height)));
			if (top_bin_count < rectSize * 0.5) continue;

			// 명암 값 분석
			// 신호등 위쪽의 명암 총 합 계산
			int top_value_count = cv::sum(hsv[2](cvRect(resultRect.x, resultRect.y-resultRect.height, resultRect.width, resultRect.height)))[0];
			if (top_value_count > rectSize * 64) continue;

			// 신호등 왼쪽의 명암 총 합 계산
			int left_value_count = cv::sum(hsv[2](cvRect(resultRect.x-resultRect.width, resultRect.y, resultRect.width, resultRect.height)))[0];
			if (left_value_count < top_value_count * 1.2) continue;

			// 신호등 오른쪽의 명암 총 합 계산
			int right_value_count = cv::sum(hsv[2](cvRect(resultRect.x+resultRect.width, resultRect.y, resultRect.width, resultRect.height)))[0];
			if (right_value_count < top_value_count * 1.2) continue;

			// 신호등 아래쪽의 명암 총 합 계산
			int bottom_value_count = cv::sum(hsv[2](cvRect(resultRect.x, resultRect.y+resultRect.height, resultRect.width, resultRect.height)))[0];
			if (bottom_value_count < top_value_count * 1.2) continue;


			greenpos += rectSize;
		}
		else {
			//rectangle(current_frame, resultRect, cv::Scalar(0,255,0), 2, 8, 0);

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

	if (_trafficLightType == EAGLETRAFFIC_LIGHTTYPE_VERTICAL) {
		if (greenpos > redpos && greenpos >= 50) return EAGLETRAFFIC_SIGN_GREEN; //cout << "GREEN SIGN" << endl;
		else if (redpos >= 50) return EAGLETRAFFIC_SIGN_RED; //cout << "RED SIGN" << endl;
		else return EAGLETRAFFIC_SIGN_NONE; //cout << "X" << endl;
	}
	else {
		if (greenpos <= -50) return EAGLETRAFFIC_SIGN_LEFT;
		else if (greenpos >= 50) return EAGLETRAFFIC_SIGN_RIGHT;
		else return EAGLETRAFFIC_SIGN_NONE;
	}
	
	
	return EAGLETRAFFIC_SIGN_NONE;
}