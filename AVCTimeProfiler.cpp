//
//  AVCTimeProfiler.cpp
//  AVC
//
//  Created by YoungJae Kwon on 11. 11. 23..
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef AVC_AVCTimeProfiler_h
#define AVC_AVCTimeProfiler_h

#include "AVCTimeProfiler.h"

#include "opencv2/opencv.hpp"

#include <math.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#ifdef MAC_OS_X_VERSION_10_7
    #include <sys/time.h>
#else
	#include <time.h>
struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

#endif

using namespace std;

static double g_time;
//윈도우용 함수
//int CC_DLL gettimeofday(struct timeval *, struct timezone *);



int AVCTimeProfiler::getCurrentTime(AVCTime *tp)
{
    if (tp)
    {
#ifdef MAC_OS_X_VERSION_10_7
		//맥 전용함수?
        gettimeofday((struct timeval *)tp,  0);
#else
	
#endif
        
    }
    return 0;
}

AVCTime AVCTimeProfiler::getTimeDiff(AVCTime *start, AVCTime *end)
{
    AVCTime timeDiff;
        
    timeDiff.tv_sec = end->tv_sec - start->tv_sec;
    timeDiff.tv_usec = end->tv_usec - start->tv_usec;
    return timeDiff;
}

void AVCTimeProfiler::begin()
{
    //getCurrentTime(&beginTime);
	g_time = (double)cv::getTickCount();
}

void AVCTimeProfiler::end()
{
//     AVCTime currentTime;
//     getCurrentTime(&currentTime);    
//     resultTime = getTimeDiff(&beginTime , &currentTime);

	g_time = (double)cv::getTickCount() - g_time;
	printf("ElapsedTime = %gms\n", g_time*1000./cv::getTickFrequency());
}
void AVCTimeProfiler::print()
{
//     double elapsed = resultTime.tv_sec*1000 + resultTime.tv_usec/1000;
//     std::cout << "ElapsedTime : " << elapsed  << "ms\n";

	
}



#endif
