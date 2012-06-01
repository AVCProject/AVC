#ifndef __AVCData_H__
#define __AVCData_H__


// ���� ������ ��ȿ�� ���� �����ֱ�
enum AVCLaneValid{
	LaneValid_LeftRight =1,
	LaneValid_Right     =2,
	LaneValid_Left      =3,
	LaneValid_None      =4
};

// ���� ��ȣ�� ���� ������
enum AVCTrafficSign{
	TrafficSign_TurnLeft    =1,
	TrafficSign_TurnRight   =2,
	TrafficSign_Stop        =3,
	TrafficSign_Go          =4
};

// struct�� ���ǵ� ������� ��Ʈ��ũ�� ���۵�. 4����Ʈ ��ŷ �� �����.
typedef struct 
{
	// ped dected : 10			val1: distance(meter)
	// Red_yellow_signal : 20	val1: ��ȣ�� ���� ����1, �ʷ�3 val2:Ⱦ�ܺ��� �Ÿ�(����)
	// left_right_signal : 30	val1:
	
	int missonID;  
	float value1;
	float value2;

//#define	EAGLETRAFFIC_SIGN_RED		1
//#define EAGLETRAFFIC_SIGN_YELLOW	2
//#define EAGLETRAFFIC_SIGN_GREEN		3
//#define EAGLETRAFFIC_SIGN_LEFT		11
//#define EAGLETRAFFIC_SIGN_RIGHT		12

	//int laneValidity;
	//float marginLeft;
	//float marginRight;
	//float angleLeft; // �̰� �ʿ�������� �𸣰ڳ�.
	//float angleRight;

	//int trafficSign;
	//int crosswalkPos;
	//int isPedDetected;
	//float pedDistance;
	//time_t timeStamp;	
} AVCData;

static void printAVCData(AVCData *data)
{
	printf("[ID]%d  val1:%0.0f\t val2:%0.0f\n",
		   data->missonID,
		   data->value1,data->value2);

	/*
	
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
		   */
}
// float���� ��Ʈ��ũ ����Ʈ ���������� ����
static float htonFloat(float fVal)
{

	unsigned long *puifVal = (unsigned long *)&fVal;    // float�� unsigned long ĳ����.

	*puifVal = htonl(*puifVal);	// ntohl(htonl) �Լ� �̿�

	return fVal;
}
// float���� host ����Ʈ ���������� ����
static float ntohFloat(float fVal)
{

	unsigned long *puifVal = (unsigned long *)&fVal;    // float�� unsigned long ĳ����.

	*puifVal = ntohl(*puifVal);	// ntohl(htonl) �Լ� �̿�

	return fVal;

}

static void convertDataToNetworkOrder(AVCData* aData)
{
	aData->missonID = htonl(aData->missonID);
	aData->value1 = htonFloat(aData->value1);
	aData->value2 = htonFloat(aData->value2);
    /*
    aData->marginLeft = htonFloat(aData->marginLeft);
    aData->marginRight = htonFloat(aData->marginRight);    
    aData->laneValidity =htonl(aData->laneValidity);

    aData->angleLeft = htonFloat(aData->angleLeft);
    aData->angleRight = htonFloat(aData->angleRight);

    aData->trafficSign = htonl(aData->trafficSign);
	aData->isPedDetected = htonl(aData->isPedDetected);
	aData->pedDistance = htonFloat(aData->pedDistance);
	//time(&data.timeStamp);
	*/
}

static void convertDataToHostOrder(AVCData* aData)
{
	aData->missonID = ntohl(aData->missonID);
	aData->value1 = ntohFloat(aData->value1);
	aData->value2 = ntohFloat(aData->value2);

	/*
	aData->marginLeft = ntohFloat(aData->marginLeft);
	aData->marginRight = ntohFloat(aData->marginRight);    
	aData->laneValidity =ntohl(aData->laneValidity);

	aData->angleLeft = ntohFloat(aData->angleLeft);
	aData->angleRight = ntohFloat(aData->angleRight);

	aData->trafficSign = ntohl(aData->trafficSign);
	aData->isPedDetected = ntohl(aData->isPedDetected);
	aData->pedDistance = ntohFloat(aData->pedDistance);
	//time(&data.timeStamp);
	*/
}

#endif