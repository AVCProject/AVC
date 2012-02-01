//
//  AVCTimeProfiler.h
//  AVC
//
//  Created by YoungJae Kwon on 11. 11. 23..
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>

// 맥용 시간구조체 동일하게 흉내
typedef struct
{
    long	tv_sec;		// seconds
    unsigned int	tv_usec;    // microSeconds
} AVCTime;

static AVCTime beginTime;
static AVCTime resultTime;

class AVCTimeProfiler
{
public:
    
    static int getCurrentTime(AVCTime *tp);
    
    static AVCTime getTimeDiff(AVCTime *start, AVCTime *end);
    static void begin();
    static void end();    
    static void print();
    
protected:

};

