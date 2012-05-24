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


#ifdef WIN32
	#pragma comment (lib, "ws2_32.lib")
	#define CLOSE(socket) closesocket(socket)
#else
	#define CLOSE(socket) close(socket)
#endif


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
	if( WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
		error_handling("WSAStartup() failed.");
		

	struct sockaddr_in serverAddr;

	serverSocket=socket(PF_INET, SOCK_STREAM, 0);   
	if(serverSocket==-1)
		error_handling("socket() error");
	
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddr.sin_port=htons(port);
	
    // prevent bind error
#ifdef WIN32
	char nSockOpt = 1;
#else
	int nSockOpt = 1;
#endif
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &nSockOpt, sizeof(nSockOpt));
    
	if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))==-1)
	{
		error_handling("bind() error");
		CLOSE(serverSocket);
	}
	
	if(listen(serverSocket, 5)==-1)
		error_handling("listen() error");
    
    
	
    return true;
	
}
void AVCNetwork::workerThread()
{
    // 연결 받기 기다리는 루프
	
	while (1)
	{
		printf("VisionServer Waiting for connection from clients...\n");
		
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
            	boost::this_thread::sleep(boost::posix_time::millisec(20));
                continue;
            }
            
            data = dataQueue.front();
            dataQueue.pop();
            
            
            convertDataToNetworkOrder(&data);
            
#ifdef WIN32
            if( send(clientSocket, (const char*)&data, sizeof(AVCData), 0) == -1 )
#else
			if( send(clientSocket, &data, sizeof(AVCData), 0) == -1 )
#endif
            {
                // 전송 에러 발생 처리
                printf("send error occured\n");
                break;
            }
            
			printAVCData(&data);
            
            
            if(isStopFlagOn)
                break;
            
        }
        if (isStopFlagOn)
            break;
        
        
		
		CLOSE(clientSocket);
  
		// 컨넥션이 끊어졌을 경우 1초 기다렸다가 다시 accept 시도
		boost::this_thread::sleep(boost::posix_time::seconds(retryTimeInterval));
	}
    
    CLOSE(serverSocket);
#ifdef WIN32
	WSACleanup();
#endif
	
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

void AVCNetwork::addToQueue(AVCData aData)
{
    dataQueue.push(aData);
}

void AVCNetwork::error_handling(const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

