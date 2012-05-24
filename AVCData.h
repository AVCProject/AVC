#ifndef __AVCData_H__
#define __AVCData_H__


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

// struct에 정의된 순서대로 네트워크상에 전송됨. 4바이트 패킹 잘 맞출것.
typedef struct 
{
	int laneValidity;
	float marginLeft;
	float marginRight;
	float angleLeft; // 이건 필요없을지도 모르겠네.
	float angleRight;

	int trafficSign;
	int crosswalkPos;
	int isPedDetected;
	float pedDistance;
	//time_t timeStamp;	
} AVCData;

static void printAVCData(AVCData *data)
{
	printf("\
[Lane]\tvalid:%d,mL:%0.0f,mR:%0.0f, angleL:%0.0f, angleR:%0.0f, \n\
[Sign]\t%d\t[Crosswalk]\t%d\n\
[Ped]\t%s, Dist:%0.0f\n",
		   data->laneValidity,
		   data->marginLeft,
		   data->marginRight,
		   data->angleLeft,
		   data->angleRight,
		   data->trafficSign,
		   data->crosswalkPos,
		   (data->isPedDetected) ? "O":"X",
		   data->pedDistance);

}
// float형을 네트워크 바이트 오더링으로 변경
static float htonFloat(float fVal)
{

	unsigned long *puifVal = (unsigned long *)&fVal;    // float를 unsigned long 캐스팅.

	*puifVal = htonl(*puifVal);	// ntohl(htonl) 함수 이용

	return fVal;
}
// float형을 host 바이트 오더링으로 변경
static float ntohFloat(float fVal)
{

	unsigned long *puifVal = (unsigned long *)&fVal;    // float를 unsigned long 캐스팅.

	*puifVal = ntohl(*puifVal);	// ntohl(htonl) 함수 이용

	return fVal;

}

static void convertDataToNetworkOrder(AVCData* aData)
{
    
    aData->marginLeft = htonFloat(aData->marginLeft);
    aData->marginRight = htonFloat(aData->marginRight);    
    aData->laneValidity =htonl(aData->laneValidity);

    aData->angleLeft = htonFloat(aData->angleLeft);
    aData->angleRight = htonFloat(aData->angleRight);

    aData->trafficSign = htonl(aData->trafficSign);
	aData->isPedDetected = htonl(aData->isPedDetected);
	aData->pedDistance = htonFloat(aData->pedDistance);
	//time(&data.timeStamp);
}

static void convertDataToHostOrder(AVCData* aData)
{

	aData->marginLeft = ntohFloat(aData->marginLeft);
	aData->marginRight = ntohFloat(aData->marginRight);    
	aData->laneValidity =ntohl(aData->laneValidity);

	aData->angleLeft = ntohFloat(aData->angleLeft);
	aData->angleRight = ntohFloat(aData->angleRight);

	aData->trafficSign = ntohl(aData->trafficSign);
	aData->isPedDetected = ntohl(aData->isPedDetected);
	aData->pedDistance = ntohFloat(aData->pedDistance);
	//time(&data.timeStamp);
}

#endif