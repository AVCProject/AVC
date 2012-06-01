#include "EagleCrosswalk.h"

EagleCrosswalk::EagleCrosswalk() {
	previous_pos = -1;
	previous_value = 0;
}

int EagleCrosswalk::decideCrosswalk(const cv::Mat &image) {
	if (image.empty()) return -1;

	float radius;
	cv::Point2f center;
	cv::Mat H = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));

	float width, height;
	
	cv::Mat roi_image;
	int y_startpos = image.rows*11/17;
	roi_image = image(cvRect(0, y_startpos, image.cols*3/4, image.rows*4/17));
	roi_image.copyTo(current_frame);

	// Convert from BGR to HSV
	cv::Mat hsvimage;
	cvtColor(current_frame, hsvimage, CV_BGR2HSV);
	vector<cv::Mat> hsv;
	split(hsvimage, hsv);
	
	//blur( hsv[1], hsv[1], cv::Size(3,3) );
	//blur( hsv[2], hsv[2], cv::Size(3,3) );
	
	dilate(hsv[1], hsv[1], H);
	erode(hsv[1], hsv[1], H);
	erode(hsv[1], hsv[1], H);

	cv::equalizeHist(hsv[1], hsv[1]);
	cv::equalizeHist(hsv[2], hsv[2]);

	//imshow("hist_sat", hsv[1]);
	//imshow("hist_val", hsv[2]);
	
	cv::Mat sat_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	cv::Mat val_bin = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	
	inRange(hsv[1], 48, 255, sat_bin); // Saturation
	inRange(hsv[2], 0, 216, val_bin); // Value
	
	floodFill(sat_bin, cv::Point(3, 3), 255, 0, cv::Scalar(5), cv::Scalar(5));
	floodFill(sat_bin, cv::Point(sat_bin.cols/2, 3), 255, 0, cv::Scalar(5), cv::Scalar(5));
	floodFill(sat_bin, cv::Point(sat_bin.cols-3, 3), 255, 0, cv::Scalar(5), cv::Scalar(5));
	floodFill(val_bin, cv::Point(3, 3), 255, 0, cv::Scalar(5), cv::Scalar(5));
	floodFill(val_bin, cv::Point(val_bin.cols/2, 3), 255, 0, cv::Scalar(5), cv::Scalar(5));
	floodFill(val_bin, cv::Point(val_bin.cols-3, 3), 255, 0, cv::Scalar(5), cv::Scalar(5));

	//imshow("sat_bin", sat_bin);
	//imshow("val_bin", val_bin);

	cv::Mat bin_com = cv::Mat::zeros(current_frame.rows, current_frame.cols, CV_8UC1);
	max(sat_bin, val_bin, bin_com); // Final
	
	dilate(bin_com, bin_com, H);
	dilate(bin_com, bin_com, H);
	dilate(bin_com, bin_com, H);
	erode(bin_com, bin_com, H);
	erode(bin_com, bin_com, H);
	erode(bin_com, bin_com, H);

	//imshow("final", bin_com);

	vector<vector<cv::Point> > contours;
	vector<cv::Vec4i> hierarchy;

	vector<CvRect> cwitems;

	int retvalue = -1;
	int retposition = -1;

	int maxvalue = 0;
	int maxposition = -1;

	findContours(bin_com, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	for (int i=0; i<contours.size(); i++) {
		if (contours[i].size() >= 100) { continue;}
		cv::RotatedRect testRect = minAreaRect(cv::Mat(contours[i]));
		CvRect testRect2 = testRect.boundingRect();
		if (testRect2.width > 80 || testRect2.height > 80) continue;
		cwitems.push_back(testRect2);
	}
	for (int i=0; i<cwitems.size(); i++) {
		vector<int> itemlist;
		itemlist.push_back(i);
		for (int j=0; j<cwitems.size(); j++) {
			if (i == j) continue;
			int flag = -1;
			for (int k=0; k<itemlist.size(); k++) {
				if (cwitems[j].x > cwitems[i].x && cwitems[j].x+cwitems[j].width < cwitems[i].x+cwitems[i].width) { flag = 1; break; }
				//if (cwitems[k].x > cwitems[j].x && cwitems[k].x+cwitems[k].width < cwitems[j].x+cwitems[j].width) { flag = 1; break; }
				if (cwitems[j].y < cwitems[i].y-(cwitems[i].height/4)) { flag = 1; break; }
				if (cwitems[j].y+cwitems[j].height > cwitems[i].y+cwitems[i].height+(cwitems[i].height/4)) { flag = 1; break; };
				break;
			}
			if (flag == -1) itemlist.push_back(j);
		}
		if (itemlist.size() < 4) continue;

		int mindist = -1;
		int maxdist = 1000000;
		int ypos = 0;
		int xpos = 0;
		int avgsize = 0;
		for (int j=0; j<itemlist.size(); j++) {
			ypos += (cwitems[itemlist[j]].y+cwitems[itemlist[j]].height);
			xpos += (cwitems[itemlist[j]].x+cwitems[itemlist[j]].width/2);
			avgsize += (cwitems[itemlist[j]].width * cwitems[itemlist[j]].height);
			
			int localmindistl = -1;
			int localmindistr = -1;
			// 좌측에서 가장 인접한 것
			for (int k=0; k<itemlist.size(); k++) {
				if (k == j) continue;
				int llm = (cwitems[itemlist[j]].x + (cwitems[itemlist[j]].width/2)) - (cwitems[itemlist[k]].x + (cwitems[itemlist[k]].width/2));
				if (llm < 0) continue;

				if (localmindistl == -1 || localmindistl > llm) localmindistl = llm;
			}
			// 우측에서 가장 인접한 것
			for (int k=0; k<itemlist.size(); k++) {
				if (k == j) continue;
				int llm = (cwitems[itemlist[k]].x + (cwitems[itemlist[k]].width/2)) - (cwitems[itemlist[j]].x + (cwitems[itemlist[j]].width/2));
				if (llm < 0) continue;

				if (localmindistr == -1 || localmindistr > llm) localmindistr = llm;
			}

			if (localmindistl != -1) {
				if (mindist == -1 || (mindist > localmindistl)) mindist = localmindistl;
				if (maxdist == 1000000 || (maxdist < localmindistl)) maxdist = localmindistl;
			}

			if (localmindistr != -1) {
				if (mindist == -1 || (mindist > localmindistr)) mindist = localmindistr;
				if (maxdist == 1000000 || (maxdist < localmindistr)) maxdist = localmindistr;
			}
		}

		xpos = xpos/itemlist.size();
		avgsize = avgsize/itemlist.size();

		//cout << mindist << " " << maxdist << endl;
		if ((maxdist-mindist) > maxdist/2) continue;
		if (xpos < image.cols/3 || xpos > image.cols*2/3) continue;

		retposition = ypos/itemlist.size();
		retposition += y_startpos; // ROI가 위치하는 실제 위치를 계산

		// 이전 결과 반영
		if (previous_pos == -1 || ((retposition - previous_pos) > -4 && (retposition - previous_pos) < 8)) {
			retvalue = previous_value;
		} else {
			retvalue = 0;
		}

		retvalue += (image.cols-(maxdist-mindist)); // 최대간격과 최소간격의 차이
		retvalue += (image.cols-abs(xpos-(image.cols/2))); // 중간 x좌표와 이미지의 중간 x좌표와의 차이
		retvalue += (avgsize*4);

		if (retvalue > maxvalue) {
			maxvalue = retvalue;
			maxposition = retposition;
		}

		for (int j=0; j<itemlist.size(); j++) {
			rectangle(current_frame, cwitems[itemlist[j]], cv::Scalar(255,255,0), 2, 8, 0);
		}
		//cout << mindist << " " << maxdist << endl;
	}
	//imshow("finalframe", current_frame);

	// 계산 결과 반영
	if (maxposition != -1) {
		previous_pos = maxposition;
		previous_value = maxvalue;
	} else {
		previous_value -= (previous_value/2 + 10);
	}

	if (previous_value <= 0) {
		previous_pos = -1;
		previous_value = 0;
	}

	return previous_value;
}