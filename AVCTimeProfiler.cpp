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


#include <math.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#ifdef MAC_OS
    #include <sys/time.h>
#else
	#include <time.h>
#endif

using namespace std;

struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

//윈도우용 함수
//int CC_DLL gettimeofday(struct timeval *, struct timezone *);



int AVCTimeProfiler::getCurrentTime(AVCTime *tp)
{
    if (tp)
    {
#ifdef MAC_OS
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
    getCurrentTime(&beginTime);
}

void AVCTimeProfiler::end()
{
    AVCTime currentTime;
    getCurrentTime(&currentTime);
    
    resultTime = getTimeDiff(&beginTime , &currentTime);
}
void AVCTimeProfiler::print()
{
    double elapsed = resultTime.tv_sec*1000 + resultTime.tv_usec/1000;
    std::cout << "ElapsedTime : " << elapsed  << "ms\n";
}



#endif
