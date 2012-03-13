//
//  AVCNetwork.cpp
//  AVC
//
//  Created by YoungJae Kwon on 11. 11. 22..
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "AVCNetwork.h"
#include <iostream>
#include <boost/thread.hpp>

//MOD:현재 맥에서만 동작하기 때문에 막아뒀음.


AVCNetwork::AVCNetwork()
{
    isStopFlagOn = false;
    
    // 초기 파라메터 설정
    retryTimeInterval = 1;
    
}
AVCNetwork::~AVCNetwork()
{
    
}


bool AVCNetwork::init(unsigned short port)
{	
	/*
	struct sockaddr_in serverAddr;
	
	serverSocket=socket(PF_INET, SOCK_STREAM, 0);   
	if(serverSocket==-1)
		error_handling("socket() error");
	p
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(port);
	
    // prevent bind error
    int nSockOpt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &nSockOpt, sizeof(nSockOpt));
    
	if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))==-1)
	{
		error_handling("bind() error");
		close(serverSocket);
	}
	
	if(listen(serverSocket, 5)==-1)
		error_handling("listen() error");
    
    
	*/
    return true;
	
}
void AVCNetwork::workerThread()
{
    // 연결 받기 기다리는 루프
	/*
	while (1)
	{
		
		printf("Unit Sizes:float%lu,int%lu,time_t%lu,",sizeof(float),sizeof(int),sizeof(time_t));
		printf("Data Size:%lu\n",sizeof(AVCData));
        
		printf("Waiting for connection from clients...\n");
		
        struct sockaddr_in clientAddr;
        socklen_t clientAddr_sz;
        clientAddr_sz=sizeof(clientAddr); 

		clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddr_sz);
		if(clientSocket == -1)
			error_handling("accept() error");
		else
			printf("Client Connected. \n");
		
		
        
		AVCData data;
		// 데이터 전송 루프
        while (1)
        {
            // 전송될 데이터를 큐에서 하나 빼옴
            if (dataQueue.size() < 1) 
            {
                printf("No data to send in queue\n");
                usleep(20000);
                continue;
            }
            
            data = dataQueue.front();
            dataQueue.pop();
            
            
            convertDataToNetworkOrder(data);
            
            if( send(clientSocket, &data, sizeof(AVCData), 0) == -1 )
            {
                // 전송 에러 발생 처리
                printf("send error occured\n");
                break;
            }
            
            printf("20ms elapsed: marginL:%f,marginR:%f,validity:%d, steer:%f, angleL:%f, angleR:%f, sign:%d\n",
                   data.marginLeft,
                   data.marginRight,
                   data.laneValidity,
                   data.steering,
                   data.angleLeft,
                   data.angleRight,
                   data.trafficSign);
            //printf("200ms elapsed: angleL:%f, angleR:%f, Offset:%f, sign:%d, TS%d\n",data.angleLeft,data.angleRight,data.lateralOffset,data.trafficSign,data.timeStamp);
            
            if(isStopFlagOn)
                break;
            
        }
        if (isStopFlagOn)
            break;
        
        
		
		close(clientSocket);
        
		// 컨넥션이 끊어졌을 경우 1초 기다렸다가 다시 accept 시도
		sleep(retryTimeInterval);
	}
    
    close(serverSocket);
	*/
}
void AVCNetwork::start()
{
    isStopFlagOn = false;
    boost::thread( &AVCNetwork::workerThread,this);
}
void AVCNetwork::stop()
{
    isStopFlagOn = true;
}
void AVCNetwork::convertDataToNetworkOrder(AVCData& aData)
{
    //time(&data.timeStamp);
	/*
    aData.marginLeft = htohFloat(aData.marginLeft);
    aData.marginRight = htohFloat(aData.marginRight);    
    aData.laneValidity =htonl(aData.laneValidity);
    aData.steering = htohFloat(aData.steering);
    
    aData.angleLeft = htohFloat(aData.angleLeft);
    aData.angleRight = htohFloat(aData.angleRight);

    aData.trafficSign = htonl(aData.trafficSign);
	*/
}
void AVCNetwork::addToQueue(AVCData aData)
{
    dataQueue.push(aData);
}

// float형을 네트워크 바이트 오더링으로 변경
float AVCNetwork::htohFloat(float fVal)
{
	/*
	unsigned long *puifVal = (unsigned long *)&fVal;    // float를 unsigned long 캐스팅.
	
	*puifVal = ntohl(*puifVal);	// ntohl(htonl) 함수 이용
	*/
	return fVal;
	
}
void AVCNetwork::error_handling(const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

