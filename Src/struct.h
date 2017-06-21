/*******************************************************************
Company:   ��˾��
Engineer:  ������
Create Date:  ��������
File  Name:  �ļ�����
Description: ������ϸ˵���˳����ļ���ɵ���Ҫ���ܣ�������ģ������Ľӿڣ����ֵ��ȡֵ��Χ�����弰������Ŀ��ơ�˳�򡢶����������ȹ�ϵ
Function List:  // ��Ҫ�����б�ÿ����¼Ӧ���������������ܼ�Ҫ˵��

Revision: �޶���Ϣ
version: �汾��
Revision Date���޶�����
Modifier: �޶�������
Additional Comments: �޶�����ϸ����

********************************************************************/

#ifndef _STRUCT_H
#define _STRUCT_H
#include "MyConst.h"
#include "rtklib.h"

//GPS/BDS�۲�����
typedef struct _tDiffObsInfoStruct
{         
	unsigned short	usWeek;						// �ز�����
	double					dSecond;					// �ز���������
	unsigned short	usNsats;					// ����ϵͳ��������Ŀ
	unsigned char 	ucSysMode[MAXNSAT];  		//����ϵͳ(GPS:1  BDS:2)  
	unsigned short 	usSvID[MAXNSAT];           	//���Ǳ��(GPS:1-32  BDS:1-14) 	
	unsigned char 	ucSnr[MAXNSAT][3]; 			//�ź�ǿ��
	unsigned char 	ucLostFlag[MAXNSAT][3];	 	// ���ջ�ʧ����־ 1:ʧ�� 
	double 			dCarrierPhase[MAXNSAT][3];   //�ز���λ�۲�ֵ L1/B1 L2/B2 L5/B3 (cycle) 
	double 			dPseudoRange[MAXNSAT][3];    //α��۲�ֵ (m)
	float  			fDoppler[MAXNSAT][3];        //�����չ۲�ֵ (Hz)
	double			dBasePos[3];				//  ��׼վ����

}TDIFFOBSINFOSTRUCT; 

// RMO??????
typedef struct _tUartRmoStruct
{
	unsigned int DHV:1;             
	unsigned int ECT:1;     
	unsigned int GGA:1;     
	unsigned int GLL:1;  

	unsigned int PCD:1;     
	unsigned int PRO:1;     
	unsigned int RAM:1;   
	unsigned int RMC:1;

	unsigned int BIN:1;         
	unsigned int ZDA:1;
	
	unsigned int RTK:1; 
}TUARTRMOSTRUCT;


// ���߽ṹ��
typedef struct _tBaseLineStruct
{
	double			dCalBaseLine;
	double			dHeading;
	double			dPitch;	
	double			dAssistHeading;
	unsigned char	bAttiSuccessFlag;
	unsigned char	ucHoldingFlag;
}TBASELINESTRUCT; 

// ���߽ṹ��
typedef struct _tBasePosStruct
{
	unsigned int 		uiUserSetFlag;
	unsigned int 		uiPosFilterFlag;
	unsigned int 		uiStationType;
	unsigned int 		uiOutputType;
	double					dFilterX;
	double					dFilterY;
	double					dFilterZ;
	double					dUserSetX;
	double					dUserSetY;
	double					dUserSetZ;
	
}TBASEPOSSTRUCT;  

typedef struct _tUartRxCacheSturct
{
	unsigned char 	ucRxBuff[MAXRAWLEN];
	unsigned short 	usLength;
}TUARTRXCACHESTRUCT;

typedef struct _tSatElevSturct
{
	double dElev[MAXSAT];
	unsigned int uiCalcElevCount[MAXSAT];
}TSATELEVSTRUCT;

typedef enum {
    NMEA = 0,
    PIX,
    JIYI,
    ZEROTECH
} RTK_OUTPUT_TYPE;

#endif



