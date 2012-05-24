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
#ifdef WIN32
	// 인클루드 순서가 중요하다. winsock2 -> windows순으로 적을것.
	#include <winsock2.h>
	#include <windows.h> //gpuMat의 min 함수와 윈도우즈 매크로가 충돌한다.

	#define NET_INVALID_SOCKET	INVALID_SOCKET
	#define NET_SOCKET_ERROR SOCKET_ERROR
	typedef int socklen_t; // Unix 타입은 unsigned int Windows 는 int

#else

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

typedef int SOCKET;
#define NET_INVALID_SOCKET	-1
#define NET_SOCKET_ERROR -1

#endif



#include <queue>
#include "AVCData.h"
#define BUF_SIZE 1024




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

    
    void workerThread();
    
    void error_handling(const char *message);

    
    bool isConnected;
    bool isStopFlagOn;
    
    SOCKET serverSocket, clientSocket;
    int retryTimeInterval;
	
	std::queue<AVCData> dataQueue;

#ifdef WIN32
	WSADATA wsaData;
#endif 
	
};


#endif
