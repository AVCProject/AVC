// AVCDataClient.cpp : Defines the entry point for the console application.
//

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


#include "AVCData.h"

#ifdef WIN32
	#pragma comment (lib, "ws2_32.lib")
	#define CLOSE(socket) closesocket(socket)
#else
	#define CLOSE(socket) close(socket)
#endif

#define RCVBUFSIZE 512   /* Size of receive buffer */

void error_handling(const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void main(int argc, char *argv[])
{
    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort = 6000;     /* Echo server port */
    char *servIP = "127.0.0.1";                    /* Server IP address (dotted quad) */
   
    char* recvBuf[RCVBUFSIZE];     /* Buffer for echo string */
   
    int bytesRcvd;
    WSADATA wsaData;                 /* Structure for WinSock setup communication */

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.0 DLL */
    {
        fprintf(stderr, "WSAStartup() failed");
        exit(1);
    }

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        error_handling("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    echoServAddr.sin_port        = htons(echoServPort); /* Server port */
    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        error_handling("connect() failed");

//     echoStringLen = strlen(echoString);          /* Determine input length */
// 
//     /* Send the string, including the null terminator, to the server */
//     if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
//         error_handling("send() sent a different number of bytes than expected");

	int bytesToRead = sizeof(AVCData);
    printf("====Connected====\n");                /* Setup to print the echoed string */
    while (1)
    {
        /* Receive up to the buffer size (minus 1 to leave space for 
           a null terminator) bytes from the sender */
        if ((bytesRcvd = recv(sock, (char*)recvBuf, bytesToRead , 0)) <= 0)
            error_handling("recv() failed or connection closed prematurely");
        
		bytesToRead -= bytesRcvd;
		printf("\nReceived:%d/%d\n",bytesRcvd,sizeof(AVCData));

		if( bytesToRead == 0 ) // 수신되어야 할 단위사이즈만큼 모두 읽어들여진 경우
		{
			AVCData* data = (AVCData*)recvBuf;
			convertDataToHostOrder(data);
			printAVCData(data);
			bytesToRead = sizeof(AVCData);
		}
    }
	printf("====Disconnected====\n\n ");  


    CLOSE(sock);
    WSACleanup();  /* Cleanup Winsock */
}