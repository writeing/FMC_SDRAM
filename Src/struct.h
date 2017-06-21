/*******************************************************************
Company:   公司名
Engineer:  作者名
Create Date:  生成日期
File  Name:  文件名称
Description: 用于详细说明此程序文件完成的主要功能，与其他模块或函数的接口，输出值、取值范围、含义及参数间的控制、顺序、独立或依赖等关系
Function List:  // 主要函数列表，每条记录应包括函数名及功能简要说明

Revision: 修订信息
version: 版本号
Revision Date：修订日期
Modifier: 修订者姓名
Additional Comments: 修订的详细描述

********************************************************************/

#ifndef _STRUCT_H
#define _STRUCT_H
#include "MyConst.h"
#include "rtklib.h"

//GPS/BDS观测数据
typedef struct _tDiffObsInfoStruct
{         
	unsigned short	usWeek;						// 关测量周
	double					dSecond;					// 关测量周内秒
	unsigned short	usNsats;					// 两个系统卫星总数目
	unsigned char 	ucSysMode[MAXNSAT];  		//卫星系统(GPS:1  BDS:2)  
	unsigned short 	usSvID[MAXNSAT];           	//卫星编号(GPS:1-32  BDS:1-14) 	
	unsigned char 	ucSnr[MAXNSAT][3]; 			//信号强度
	unsigned char 	ucLostFlag[MAXNSAT][3];	 	// 接收机失锁标志 1:失锁 
	double 			dCarrierPhase[MAXNSAT][3];   //载波相位观测值 L1/B1 L2/B2 L5/B3 (cycle) 
	double 			dPseudoRange[MAXNSAT][3];    //伪距观测值 (m)
	float  			fDoppler[MAXNSAT][3];        //多普勒观测值 (Hz)
	double			dBasePos[3];				//  基准站坐标

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


// 基线结构体
typedef struct _tBaseLineStruct
{
	double			dCalBaseLine;
	double			dHeading;
	double			dPitch;	
	double			dAssistHeading;
	unsigned char	bAttiSuccessFlag;
	unsigned char	ucHoldingFlag;
}TBASELINESTRUCT; 

// 基线结构体
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



