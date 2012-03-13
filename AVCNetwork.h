//
//  AVCNetwork.h
//  AVC
//
//  Created by YoungJae Kwon on 11. 11. 22..
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef AVC_AVCNetwork_h
#define AVC_AVCNetwork_h


// 네트워크 관련 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef MAC_OS_X_VERSION_10_6
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#else
//#include <windows.h>
#include <winsock2.h>
#endif



#include <queue>

#define BUF_SIZE 1024

// 차선 상태의 유효성 변수 날려주기
enum AVCLaneValid{
    LaneValid_LeftRight =1,
    LaneValid_Right     =2,
    LaneValid_Left      =3,
    LaneValid_None      =4
};

// 현재 신호등 정보 보내기
enum AVCTrafficSign{
    TrafficSign_TurnLeft    =1,
    TrafficSign_TurnRight   =2,
    TrafficSign_Stop        =3,
    TrafficSign_Go          =4
};

typedef struct 
{
    
    float marginLeft;
    float marginRight;
    int laneValidity;
    float steering; // 조향각
    float angleLeft; // 이건 필요없을지도 모르겠네.
	float angleRight;
    
	int trafficSign;
	//time_t timeStamp; // time_t == long
	
} AVCData;


class AVCNetwork
{
public:
    AVCNetwork();
    ~AVCNetwork();
    
    bool init(unsigned short port);
    void start();
    void stop();
    void addToQueue(AVCData data);
    
protected:
    void convertDataToNetworkOrder(AVCData& aData);
    
    void workerThread();
    
    float htohFloat(float fVal);
    void error_handling(const char *message);

    
    bool isConnected;
    bool isStopFlagOn;
    
    int serverSocket, clientSocket;
    int retryTimeInterval;
	
	std::queue<AVCData> dataQueue;
};


#endif
