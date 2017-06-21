#include "main.h"
#include "usart.h"
#include "global.h"
#include "rtklib.h"
#include "TimeConvert.h"
#include "MyConst.h"

//////////////////////////////////////////////////////////////////////////
////////////////////////////////串口数据发送//////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void UartSend(UART_HandleTypeDef *pUartHandle, char *pcData, unsigned short usLength)
{
	int i = 0;
	
	for(i=0; i<usLength; i++)
	{
		UsartSendData(pUartHandle, pcData[i]);
		while (__HAL_UART_GET_FLAG(pUartHandle, UART_FLAG_TC) == RESET)
		{
	
		}
	}
		return;	
}
unsigned int uiStrLenErrCount = 0;
unsigned short strlen1(const char *str)
{
	unsigned short usLength = 0;
	
	while(*str++)
	{
		++usLength;	
		if(usLength>=UART_RECV_BUF_LENTH)
		{
			uiStrLenErrCount++;
			return 0;
		}
	}
	return usLength;
}


//////////////////////////////////////////////////////////////////////////
/////////////////////////////????????BUFF////////////////////////
//////////////////////////////////////////////////////////////////////////  
//#pragma CODE_SECTION(AddFieldChar,"sect_ECODE_II");
void AddFieldChar (char *pcBuffer, unsigned short *pusIndex, char *pcAddString)
{
    unsigned short usStringLength = 0;
	
	// ???????
    usStringLength 	= strlen1(pcAddString);
    // ??????Buffer    
    memcpy (&pcBuffer[*pusIndex], pcAddString, usStringLength);
	
    *pusIndex 		+= usStringLength;
}

//////////////////////////////////////////////////////////////////////////
////////////////////////////////?????////////////////////////////////
//////////////////////////////////////////////////////////////////////////  
//#pragma CODE_SECTION(CalculateBDCheckSum,"sect_ECODE_II"); 
unsigned char CalculateBDCheckSum (char *pcCheckSumFormat)
{
    unsigned short Index;
    unsigned char BDCheckSum = 0;
    
    for (Index = 1; pcCheckSumFormat[Index] != '*'; Index++)
    {    
        BDCheckSum ^= pcCheckSumFormat[Index];
    }
    return BDCheckSum;
} 

//////////////////////////////////////////////////////////////////////////
///////////////////////////??????ASCII???////////////////////////
//////////////////////////////////////////////////////////////////////////  
//#pragma CODE_SECTION(BDAddCheckSum,"sect_ECODE_II");  
void BDAddCheckSum ( char *pcSendBuf, unsigned short *pusSendIndex )
{
    unsigned char ucCheckSum;
    char cNewFormatCheckSum[10];


    AddFieldChar (pcSendBuf, pusSendIndex, BD_SENTENCE_ASTERISK);
    ucCheckSum 					= CalculateBDCheckSum (pcSendBuf);
	
    // ?????4bit?ASCII?
    cNewFormatCheckSum[0] = ((ucCheckSum & 0xf0) >> 4) + '0';
    if (cNewFormatCheckSum[0] > '9')
    {
        cNewFormatCheckSum[0] 	+= 'A' - ('9' + 1);
    }
    cNewFormatCheckSum[1] 		= '\0';
    AddFieldChar (pcSendBuf, pusSendIndex, cNewFormatCheckSum);
	
    // ?????4bit?ASCII?
    cNewFormatCheckSum[0] = (ucCheckSum & 0xf) + '0';
    if (cNewFormatCheckSum[0] > '9')
    {
        cNewFormatCheckSum[0] 	+= 'A' - ('9' + 1);
    }
    cNewFormatCheckSum[1] 		= '\0';    
    AddFieldChar (pcSendBuf, pusSendIndex, cNewFormatCheckSum);
} 

//////////////////////////////////////////////////////////////////////////
////////////////////////////////发送RTK结果///////////////////////////////
//////////////////////////////////////////////////////////////////////////  
void SendDataRtk( UART_HandleTypeDef *huart,unsigned int uiRtkTic  )
{	
	rtk_t *ptRtk = &g_tRtk;
	int 		iGpsWeek;
	double 	dRoverSecond;
	
	if (g_usRawRoverFlag == 0)	
		return;
	
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
		
	g_usSendIndex = 0;
	memset( g_cSendBuf,0,sizeof(g_cSendBuf));

	 // ????$
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_START);
	// ??????
 
    AddFieldChar (g_cSendBuf, &g_usSendIndex, "RTK");

    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
   
	
	// ????
	if (ptRtk->sol.ratio<0)
	{
		AddFieldChar (g_cSendBuf, &g_usSendIndex, "NONE");
	}
	else if(1==ptRtk->sol.stat)
	{
		
		if(ptRtk->sol.ns<10)
		{
			AddFieldChar (g_cSendBuf, &g_usSendIndex, "FLOAT");
		}
		else
		{
			AddFieldChar (g_cSendBuf, &g_usSendIndex, "FIX");
		}
	}
	else if(2==ptRtk->sol.stat)
	{
		AddFieldChar (g_cSendBuf, &g_usSendIndex, "FLOAT");
	}
	else if(5==ptRtk->sol.stat)
	{
		AddFieldChar (g_cSendBuf, &g_usSendIndex, "SINGLE");
	}
	else
	{
		AddFieldChar (g_cSendBuf, &g_usSendIndex, "NONE");
	}

    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);


	// ??TIC
    sprintf (g_cFieldBuf, "%.3f",dRoverSecond);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

		// 高程异常
		sprintf (g_cFieldBuf, "%.3f", g_tRtk.opt.elmin);
		AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
		AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
		sprintf (g_cFieldBuf, "%02d", g_usSatNum[0]);
		AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
		AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
		
		sprintf (g_cFieldBuf, "%02d", g_usSatNum[1]);
		AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
		AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
		
		sprintf (g_cFieldBuf, "%02d", g_usSatNum[2]);
		AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
		AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
		
		sprintf (g_cFieldBuf, "%02d,%02d,%02d,%02d,%02d", g_usSatNum[4],g_usSatNum[5],g_usSatNum[6],g_usSatNum[7],g_usSatNum[8]);
		AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
		AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
		
		memset(g_usSatNum,0,sizeof(g_usSatNum));


    sprintf(g_cFieldBuf,"%.6f",ptRtk->sol.rr[0]);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

	sprintf(g_cFieldBuf,"%.6f",ptRtk->sol.rr[1]);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

	sprintf(g_cFieldBuf,"%.6f",ptRtk->sol.rr[2]);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
		
	    sprintf(g_cFieldBuf,"%.6f",ptRtk->rb[0]);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

	sprintf(g_cFieldBuf,"%.6f",ptRtk->rb[1]);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

	sprintf(g_cFieldBuf,"%.6f",ptRtk->rb[2]);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
    
	// ns
    sprintf (g_cFieldBuf, "%02d",ptRtk->sol.ns);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
		
		// ns
    sprintf (g_cFieldBuf, "%05d",uiRtkTic);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
		
    
    // age
    sprintf (g_cFieldBuf, "%.3f",ptRtk->sol.age);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

	// ratio
    sprintf (g_cFieldBuf, "%.3f",ptRtk->sol.ratio);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

	//16 ???
    BDAddCheckSum (g_cSendBuf, &g_usSendIndex);
    g_cSendBuf[g_usSendIndex++] 	= 0x0D; 
    g_cSendBuf[g_usSendIndex++] 	= 0x0A; 
    g_cSendBuf[g_usSendIndex] 		= '\0'; 	
		UartSend(huart, g_cSendBuf, g_usSendIndex);
    //UartSend(g_cSendBuf, g_usSendIndex,UART_PORT_DT2); 
	//UartSend(g_cSendBuf, g_usSendIndex,UART_PORT_FPGA);
 
}

//////////////////////////////////////////////////////////////////////////
////////////////////////////////发送GGA结果///////////////////////////////
//////////////////////////////////////////////////////////////////////////  
unsigned int uiTMEP=0;
void SendDataGGA( UART_HandleTypeDef *huart)
{	
	rtk_t *ptRtk = &g_tRtk;
	int 		iGpsWeek;
	double 	dRoverSecond,dSec;
	unsigned short 	usIntSec,usYear,usMonth,usDay,usHour,usMintue;
	unsigned int 		uiDecSec;
	
	unsigned short 	usNavMode,usUtcTimeValid,usDegrees;   
	double 					dMinute,dValue,dHighDiff,dPos[3];
	
	uiTMEP++;	
	if (g_usRawRoverFlag == 0)	
		return;
	
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);

	ecef2pos(ptRtk->sol.rr,dPos);

	g_usSendIndex = 0;
	memset( g_cSendBuf,0,sizeof(g_cSendBuf));

	// 类型标志
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_START);

	// 数据帧头标志 
	AddFieldChar (g_cSendBuf, &g_usSendIndex, "GPGGA");
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

	// 定位时间
	dRoverSecond += 0.001;
	GpsTimeToUtcTime(&g_tGpsUtcTimePara,iGpsWeek, dRoverSecond, &usYear, &usMonth, &usDay, &usHour, &usMintue, &dSec );	
	usIntSec 		= (unsigned int)dSec;
  uiDecSec 		= (unsigned int)((dSec - usIntSec)* 100); 
	
	sprintf(g_cFieldBuf,"%02d%02d%02d.%02d",usHour,usMintue,usIntSec,uiDecSec);	
	AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
  AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// 纬度
	dValue 			= fabs(dPos[0])*180/PI;
	usDegrees 	= (unsigned short)dValue;
	dMinute 		= (dValue-usDegrees) * 60.0;
	dValue 			= (dMinute - (unsigned short)dMinute) * 10000000;
	
	sprintf(g_cFieldBuf,"%d%02d.%07d",usDegrees,(unsigned short)dMinute,(unsigned int)dValue);
  AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
  AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// 纬度方向
  if(dPos[1]<0.0)    
	{
        AddFieldChar (g_cSendBuf, &g_usSendIndex, "S");
	}
  else                        
	{
        AddFieldChar (g_cSendBuf, &g_usSendIndex, "N");
	}
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// 经度
	dValue 			= fabs(dPos[1])*180/PI;
	usDegrees 	= (unsigned short)dValue;
	dMinute 		= (dValue-usDegrees) * 60.0;
	dValue 			= (dMinute - (unsigned short)dMinute) * 10000000;
	
	sprintf(g_cFieldBuf,"%d%02d.%07d",usDegrees,(unsigned short)dMinute,(unsigned int)dValue);
  AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
  AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// 经度方向
  if(dPos[1]<0.0)    
	{
        AddFieldChar (g_cSendBuf, &g_usSendIndex, "W");
	}
  else                        
	{
        AddFieldChar (g_cSendBuf, &g_usSendIndex, "E");
	}
  AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// 定位标志：0：未定位；1：FIX；2：float；4：dgps；5：single
	if(1==ptRtk->sol.stat)
	{
		
		if(ptRtk->sol.ns<10)
		{
			sprintf (g_cFieldBuf, "%d", 6);
		}
		else
		{
			sprintf (g_cFieldBuf, "%d", 1);
		}
	}
	else if(ptRtk->sol.stat>1 && ptRtk->sol.stat<5)
	{
		sprintf (g_cFieldBuf, "%d", 1);
	}
	else if(ptRtk->sol.stat==5)
	{
		sprintf (g_cFieldBuf, "%d", 1);
	}
	else
	{
		sprintf (g_cFieldBuf, "%d", 0);
	}	
    AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
    AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// 视野内卫星数
	sprintf (g_cFieldBuf, "%02d", g_tRawRover.obs.n);
	AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// HDOP
	sprintf (g_cFieldBuf, "%.1f", 0.0);
	AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

	// 海拔高度
	sprintf (g_cFieldBuf, "%.6lf", dPos[2]);
	AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// 海拔高度单位
	AddFieldChar (g_cSendBuf, &g_usSendIndex, "M");
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// 高程异常
	//sprintf (g_cFieldBuf, "%.2f", ptRtk->sol.ratio);
	//AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

	// 高程异常单位
	AddFieldChar (g_cSendBuf, &g_usSendIndex, "M");
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// 差分数据寿命
	sprintf (g_cFieldBuf, "%.2f", ptRtk->sol.age);	
	AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// 差分卫星数
	sprintf (g_cFieldBuf, "%02d", ptRtk->sol.ns);
	AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
	//AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	
	
	
	
			
	// 校验和
	BDAddCheckSum (g_cSendBuf, &g_usSendIndex);
	g_cSendBuf[g_usSendIndex++] 	= 0x0D; 
	g_cSendBuf[g_usSendIndex++] 	= 0x0A; 
	g_cSendBuf[g_usSendIndex] 		= '\0'; 
	
	//UartSend(huart, g_cSendBuf, g_usSendIndex);
			
	//memcpy(g_ucSendBuf,g_cSendBuf,g_usSendIndex);	
	
	//while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY)
  {
  }

	
	//if(HAL_UART_Transmit_DMA(huart, (uint8_t*)g_ucSendBuf, g_usSendIndex)!= HAL_OK)
  {
    //Error_Handler();
  }
	
 
}

//////////////////////////////////////////////////////////////////////////
////////////////////////////////发送ZDA结果///////////////////////////////
//////////////////////////////////////////////////////////////////////////  
void SendDataZDA( UART_HandleTypeDef *huart)
{	
	rtk_t *ptRtk = &g_tRtk;
	int 		iGpsWeek;
	double 	dRoverSecond,dSec;
	unsigned short 	usIntSec,usYear,usMonth,usDay,usHour,usMintue;
	unsigned int 		uiDecSec;
	
	unsigned short 	usNavMode,usUtcTimeValid,usDegrees;   
	double 					dMinute,dValue,dHighDiff,dPos[3];
	
		
	if (g_usRawRoverFlag == 0)	
		return;
	
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);


	g_usSendIndex = 0;
	memset( g_cSendBuf,0,sizeof(g_cSendBuf));

	// 类型标志
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_START);

	// 数据帧头标志 
	AddFieldChar (g_cSendBuf, &g_usSendIndex, "GPZDA");
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

	// 定位时间
	dRoverSecond += 0.001;
	GpsTimeToUtcTime(&g_tGpsUtcTimePara,iGpsWeek, dRoverSecond, &usYear, &usMonth, &usDay, &usHour, &usMintue, &dSec );	
			
	usIntSec 		= (unsigned int)dSec;
  uiDecSec 		= (unsigned int)((dSec - usIntSec)* 100); 
	
	sprintf(g_cFieldBuf,"%02d%02d%02d.%02d",usHour,usMintue,usIntSec,uiDecSec);	
	AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
  AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// 日期
	
	sprintf(g_cFieldBuf,"%02d,%02d,%04d",usDay,usMonth,usYear);	
	AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
  AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

	
	
	// 时区
	AddFieldChar (g_cSendBuf, &g_usSendIndex, "00");
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	AddFieldChar (g_cSendBuf, &g_usSendIndex, "00");
	//AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
			
	// 校验和
	BDAddCheckSum (g_cSendBuf, &g_usSendIndex);
	g_cSendBuf[g_usSendIndex++] 	= 0x0D; 
	g_cSendBuf[g_usSendIndex++] 	= 0x0A; 
	g_cSendBuf[g_usSendIndex] 		= '\0'; 
	
	UartSend(huart, g_cSendBuf, g_usSendIndex);
 
}

//////////////////////////////////////////////////////////////////////////
////////////////////////////////发送VTG结果///////////////////////////////
//////////////////////////////////////////////////////////////////////////  
void SendDataVTG( UART_HandleTypeDef *huart)
{	
	rtk_t *ptRtk = &g_tRtk;
	double 						dPos[3],dVelEnu[3],dDirection,dHorizenSpeed;
	static double 		sdPrevDirection=0.0;
	
	
		
	if (g_usRawRoverFlag == 0)	
		return;

	// 计算ENU速度
	ecef2pos(ptRtk->sol.rr,dPos);
	ecef2enu(dPos,ptRtk->sol.rr+3,dVelEnu);
	
	// 计算对地速度
	dHorizenSpeed = sqrt(dVelEnu[0]*dVelEnu[0] + dVelEnu[1]*dVelEnu[1] );


	// 计算对地速度方向
	if (dHorizenSpeed>=2.0) 
	{
		dDirection		= atan2(dVelEnu[0], dVelEnu[1])*RAD2DEG;
		if (dDirection<0.0)
			dDirection	+= 360.0;
		sdPrevDirection	= dDirection;
	}
	else dDirection 	= sdPrevDirection;
	

	g_usSendIndex = 0;
	
	memset( g_cSendBuf,0,sizeof(g_cSendBuf));

	// 类型标志
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_START);

	// 数据帧头标志 
	AddFieldChar (g_cSendBuf, &g_usSendIndex, "GPVTG");
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	if(ptRtk->sol.stat==0)
	{
		AddFieldChar (g_cSendBuf, &g_usSendIndex, ",,,,,,,,N");
	}
	else
	{
		sprintf(g_cFieldBuf,"%.2f,T,,M,%.3f,N,%.3f,K,A",dDirection,dHorizenSpeed/0.514444444,dHorizenSpeed*3.6);	
		AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
	}
				
	// 校验和
	BDAddCheckSum (g_cSendBuf, &g_usSendIndex);
	g_cSendBuf[g_usSendIndex++] 	= 0x0D; 
	g_cSendBuf[g_usSendIndex++] 	= 0x0A; 
	g_cSendBuf[g_usSendIndex] 		= '\0'; 
	
	//UartSend(huart, g_cSendBuf, g_usSendIndex);
 
}

static void setcs(unsigned char *buff, int len)
{
    unsigned char cka=0,ckb=0;
    int i;
    
    for (i=2;i<len-2;i++) {
        cka+=buff[i]; ckb+=cka;
    }
    buff[len-2]=cka;
    buff[len-1]=ckb;
}
unsigned char GetUBXSolStat()
{
	rtk_t *ptRtk = &g_tRtk;
	if(1==ptRtk->sol.stat)
	{
		
		if(ptRtk->sol.ns<10)
		{
			return 4;
		}
		else
		{
			return 5;
		}
	}
	else if(ptRtk->sol.stat>1 && ptRtk->sol.stat<5)
	{
		return 4;
	}
	else if(ptRtk->sol.stat==5)
	{
		return 3;
	}
	else
	{
		return 0;
	}

}
//////////////////////////////////////////////////////////////////////////
///////////////////////////////发送POSLLH结果/////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void SendDataPOSLLH( UART_HandleTypeDef *huart )
{
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n;
	int 						iGpsWeek;
	double 					dRoverSecond,dPos[3];
		
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	ecef2pos(ptRtk->sol.rr,dPos);
	
	// header
	*q++=0xB5;
	*q++=0x62;
	// class
	*q++=0x01;
	// ID
	*q++=0x02;
	
	// length
	q+=2;
	
	// GPS时 ms
	setU4(q,(unsigned int)(dRoverSecond*1000+0.5)); 					q+=4;
	setI4(q,(int)(dPos[1]*R2D*1e7)); 								q+=4;
	setI4(q,(int)(dPos[0]*R2D*1e7)); 								q+=4;
	setI4(q,(int)(dPos[2]*1e3)); 								q+=4;
	setI4(q,(int)(dPos[2]*1e3)); 								q+=4;
	setU4(q,(unsigned int)(g_dDop[2]*1000+0.5)); 																q+=4;
	setU4(q,(unsigned int)(g_dDop[3]*1000+0.5)); 																q+=4;
	
	n=(int)(q-g_cSendBuf)+2;    
	setU2(g_cSendBuf+4,(unsigned short)(n-8));
	setcs(g_cSendBuf,n);

	g_usSendIndex = n;
	UartSend(huart, g_cSendBuf, n);
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////////发送SOL结果/////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void SendDataSOL( UART_HandleTypeDef *huart )
{
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n,ucFixStatus;
	int 						iGpsWeek,iNsec;
	unsigned int		uiMsec;
	double 					dRoverSecond,dPos[3];
		
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	ecef2pos(ptRtk->sol.rr,dPos);
	
	// header
	*q++=0xB5;
	*q++=0x62;
	// class
	*q++=0x01;
	// ID
	*q++=0x06;
	
	// length
	q+=2;
	
	// GPS时 ms
	uiMsec 	= (unsigned int)(dRoverSecond*1000+0.5);
	iNsec 	= (int)(dRoverSecond*1000.0 - uiMsec)*1e6;
	setU4(q,uiMsec); 														q+=4;
	setI4(q,iNsec); 														q+=4;
	setI2(q,iGpsWeek); 													q+=2;
	setU1(q,GetUBXSolStat()); 									q+=1;
	
	if ( (ptRtk->sol.stat >= 1)&&(ptRtk->sol.stat <= 4) )
	{
		ucFixStatus = 0xDF;
	}
	else if (ptRtk->sol.stat==5)
	{
		ucFixStatus = 0xDD;
	}
	else
	{
		ucFixStatus = 0xD0;
	}
	setU1(q,ucFixStatus); 											q+=1;
	
	setI4(q,(int)(ptRtk->sol.rr[0]*100)); 			q+=4;
	setI4(q,(int)(ptRtk->sol.rr[1]*100)); 			q+=4;
	setI4(q,(int)(ptRtk->sol.rr[2]*100)); 			q+=4;
	setU4(q,(unsigned int)(g_dDop[1]*100+0.5)); 																q+=4;
	
	setI4(q,(int)(ptRtk->sol.rr[3]*100)); 																q+=4;
	setI4(q,(int)(ptRtk->sol.rr[4]*100)); 																q+=4;
	setI4(q,(int)(ptRtk->sol.rr[5]*100)); 																q+=4;
	setU4(q,(unsigned int)(0.5)); 																q+=4;
	
	setU2(q,(unsigned short)(g_dDop[1]*100+0.5)); 																q+=2;
	
	setU1(q,0); 																q+=1;
	setU1(q,ptRtk->sol.ns + 5); 																q+=1;
	setU1(q,0); 																q+=1;
	setU1(q,0); 																q+=1;
	setU1(q,0); 																q+=1;
	setU1(q,0); 																q+=1;

	
	n=(int)(q-g_cSendBuf)+2;    
	setU2(g_cSendBuf+4,(unsigned short)(n-8));
	setcs(g_cSendBuf,n);
	g_usSendIndex = n;
	
	UartSend(huart, g_cSendBuf, n);
}
//////////////////////////////////////////////////////////////////////////
///////////////////////////////?? UBX RTK??//////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void SendRTKUbxData(UART_HandleTypeDef *huart)
{
	rtk_t *ptRtk = &g_tRtk;
	unsigned char *q=g_cSendBuf,n;
	int 					iGpsWeek;
	double 					dRoverSecond;		
	// header
	*q++=0xB5;
	*q++=0x62;
	// class
	*q++=0x42;//B
	// ID
	*q++=0x52;//R
	// length
	q+=2;
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);

	setU2(q,(unsigned int)(iGpsWeek)); 						q+=2;
	setU4(q,(unsigned int)(dRoverSecond*1000+0.5)); 		q+=4;	
	setR8(q,(double)(ptRtk->rb[0])); 	 	q+=8;
	setR8(q,(double)(ptRtk->rb[1])); 	 	q+=8;
	setR8(q,(double)(ptRtk->rb[2])); 	 	q+=8;	
	setR8(q,(double)(ptRtk->sol.rr[0])); 	q+=8;
	setR8(q,(double)(ptRtk->sol.rr[1])); 	q+=8;
	setR8(q,(double)(ptRtk->sol.rr[2])); 	q+=8;
	setU1(q,ptRtk->sol.stat); 		q+=1;
	setU1(q,ptRtk->sol.ns); 		q+=1;

	n=(int)(q-g_cSendBuf)+2;    
	setU2(g_cSendBuf+4,(unsigned short)(n-8));
	setcs(g_cSendBuf,n);

	g_usSendIndex = n;
	UartSend(huart, g_cSendBuf, n);
}

void SendRoverView(UART_HandleTypeDef *huart,unsigned int uiRtkTic)
{
	static unsigned int fistFixTime = 0;
	static unsigned char fistFixFlag = 0;
	rtk_t 					*ptRtk = &g_tRtk;
	int 					iGpsWeek;
	unsigned char *q=g_cSendBuf,n;
	double 					dRoverSecond;	
	
	// header
	*q++=0xB5;
	*q++=0x62;
	// class
	*q++=0x01;
	// ID
	*q++=0x18;
	// length
	q+=2;

	/*?????*/
	// dRoverSecond = time2gpst(g_tRawBase.time,&iGpsWeek);
	setU2(q,(unsigned int)(g_tRevBaseInfo.usWeek)); 						q+=2;
	setU4(q,(unsigned int)(g_tRevBaseInfo.dSecond*1000+0.5));			q+=4;

	setR8(q,(double)(ptRtk->rb[0])); 		q+=8;
	setR8(q,(double)(ptRtk->rb[1])); 		q+=8;
	setR8(q,(double)(ptRtk->rb[2])); 		q+=8;

	setU1(q,g_tRawBase.obs.n); 			q+=1;

	/*?????*/
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	setU2(q,(unsigned int)(iGpsWeek)); 						q+=2;
	setU4(q,(unsigned int)(dRoverSecond*1000+0.5)); 		q+=4;

	setR8(q,(double)(ptRtk->sol.rr[0])); q+=8;
	setR8(q,(double)(ptRtk->sol.rr[1])); q+=8;
	setR8(q,(double)(ptRtk->sol.rr[2])); q+=8;
	setR8(q,(double)(ptRtk->sol.rr[3])); q+=8;
	setR8(q,(double)(ptRtk->sol.rr[4])); q+=8;
	setR8(q,(double)(ptRtk->sol.rr[5])); q+=8;

	setR4(q,(float)(ptRtk->sol.age)); 	q+=4;
	setR4(q,(float)(ptRtk->sol.ratio)); q+=4;

#if 1
	if(1==ptRtk->sol.stat)
	{
		
		if(ptRtk->sol.ns<10)
		{
			setU1(q,SOLQ_FLOAT);
		}
		else
		{
			setU1(q,ptRtk->sol.stat);
		}
	}
	else
#endif		
	{
		setU1(q,ptRtk->sol.stat);	
	}
	q+=1;
	
	setU1(q,g_tRawRover.obs.n); 	q+=1;
	setU1(q,ptRtk->sol.ns); 		q+=1;

	setU4(q,(unsigned int)uiRtkTic); 	q+=4;
	//setU4(q,(unsigned int)fistFixTime); q+=4;

	n=(int)(q-g_cSendBuf)+2;    
	setU2(g_cSendBuf+4,(unsigned short)(n-8));
	setcs(g_cSendBuf,n);
	g_usSendIndex = n;

	UartSend(huart, g_cSendBuf, n);
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////////发送STATUS结果/////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void SendDataSTATUS( UART_HandleTypeDef *huart )
{
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n,ucFixStatus;
	int 						iGpsWeek,iNsec;
	unsigned int		uiMsec;
	double 					dRoverSecond,dPos[3];
	unsigned int Ms;
	static unsigned int fistFixTime = 0;
	static unsigned char fistFixFlag = 0;
		
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	ecef2pos(ptRtk->sol.rr,dPos);
	
	// header
	*q++=0xB5;
	*q++=0x62;
	// class
	*q++=0x01;
	// ID
	*q++=0x03;
	
	// length
	q+=2;
	
	// GPS时 ms
	uiMsec 	= (unsigned int)(dRoverSecond*1000+0.5);
	setU4(q,uiMsec); 														q+=4;
	
	setU1(q,GetUBXSolStat()); 									q+=1;
	
	if ( (ptRtk->sol.stat >= 1)&&(ptRtk->sol.stat <= 4) )
	{
		ucFixStatus = 0xDF;
	}
	else if (ptRtk->sol.stat==5)
	{
		ucFixStatus = 0xDD;
	}
	else
	{
		ucFixStatus = 0xD0;
	}

	setU1(q,ucFixStatus); 											q+=1;
	setU1(q,0); 																q+=1;
	setU1(q,0x20); 																q+=1;
 
	Ms = HAL_GetTick();	

	if(ptRtk->sol.stat > 0 && fistFixFlag ==0)
	{
		fistFixTime = Ms;
		fistFixFlag = 1;
	}

	setU4(q,fistFixTime); 																q+=4;
	setU4(q,Ms); 																q+=4;
		
	n=(int)(q-g_cSendBuf)+2;    
	setU2(g_cSendBuf+4,(unsigned short)(n-8));
	setcs(g_cSendBuf,n);

	g_usSendIndex = n;
	UartSend(huart, g_cSendBuf, n);
}


//////////////////////////////////////////////////////////////////////////
///////////////////////////////发送VELNED结果/////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void SendDataVELNED( UART_HandleTypeDef *huart )
{
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n;
	int 						iGpsWeek;
	double 					dRoverSecond,dPos[3],dVelEnu[3],dDirection,dSpeed=0,dGroundSpeed=0;
	double  dHorizenSpeed3D,dHorizenSpeed2D;
	static double 		sdPrevDirection=0.0;
		
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	ecef2pos(ptRtk->sol.rr,dPos);
	ecef2enu(dPos,ptRtk->sol.rr+3,dVelEnu);
// 计算对地速度
	dHorizenSpeed2D = sqrt(dVelEnu[0]*dVelEnu[0] + dVelEnu[1]*dVelEnu[1] );
	dHorizenSpeed3D = sqrt(dVelEnu[0]*dVelEnu[0] + dVelEnu[1]*dVelEnu[1] + dVelEnu[2]*dVelEnu[2]);
	// 计算速度方向
	if (dHorizenSpeed2D >= 1.0) 
	{
		dDirection		= atan2(dVelEnu[0], dVelEnu[1])*RAD2DEG;
		if (dDirection<0.0)
			dDirection	+= 360.0;
		sdPrevDirection	= dDirection;
	}
	else dDirection 	= sdPrevDirection;	
	
	// header
	*q++=0xB5;
	*q++=0x62;
	// class
	*q++=0x01;
	// ID
	*q++=0x12;
	
	// length
	q+=2;
	
	// GPS时 ms
	setU4(q,(unsigned int)(dRoverSecond*1000+0.5)); 					q+=4;
	setI4(q,(int)round(dVelEnu[1]*1e2)); 													q+=4;
	setI4(q,(int)round(dVelEnu[0]*1e2)); 													q+=4;
	setI4(q,(int)round(0-dVelEnu[2]*1e2)); 												q+=4;
	
	setU4(q,(unsigned int)round(dHorizenSpeed3D*1e2)); 																q+=4;
	setU4(q,(unsigned int)round(dHorizenSpeed2D*1e2)); 																q+=4;
	setI4(q,0); 												q+=4;
	
	setU4(q,0); 																q+=4;
	setU4(q,0); 																q+=4;
	
	n=(int)(q - g_cSendBuf)+2;    
	setU2(g_cSendBuf+4,(unsigned short)(n-8));
	setcs(g_cSendBuf,n);

	g_usSendIndex = n;
	UartSend(huart, g_cSendBuf, n);
}


//////////////////////////////////////////////////////////////////////////
///////////////////////////////发送TIMEUTC结果/////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void SendDataTIMEUTC( UART_HandleTypeDef *huart )
{
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n;
	int 						iGpsWeek;
	double 					dRoverSecond,dSec;
	unsigned short 	usIntSec,usYear,usMonth,usDay,usHour,usMintue;
	unsigned int 		uiDecSec;	
		
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	
	// 定位时间
	//dRoverSecond += 0.001;
	GpsTimeToUtcTime(&g_tGpsUtcTimePara,iGpsWeek, dRoverSecond, &usYear, &usMonth, &usDay, &usHour, &usMintue, &dSec );	
	usIntSec 		= (unsigned int)dSec;
  uiDecSec 		= (unsigned int)((dSec - usIntSec)); 	
	
	// header
	*q++=0xB5;
	*q++=0x62;
	// class
	*q++=0x01;
	// ID
	*q++=0x21;
	
	// length
	q+=2;
	
	// GPS时 ms
	setU4(q,(unsigned int)(dRoverSecond*1000+0.5)); 					q+=4;
	
	setU4(q,(unsigned int)(0.5)); 																q+=4;
	setI4(q,(int)(0.5)); 												q+=4;	
	setU2(q,usYear); 																q+=2;
	setU1(q,usMonth); 																q+=1;
	setU1(q,usDay); 																q+=1;
	setU1(q,usHour); 																q+=1;
	setU1(q,usMintue); 																q+=1;
	setU1(q,usIntSec); 																q+=1;
	
	
	setU1(q,0x37); 																q+=1;
	
	n=(int)(q-g_cSendBuf)+2;    
	setU2(g_cSendBuf+4,(unsigned short)(n-8));
	setcs(g_cSendBuf,n);

	g_usSendIndex = n;
	UartSend(huart, g_cSendBuf, n);
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////////发送DOP结果/////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void SendDataDOP( UART_HandleTypeDef *huart )
{
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n;
	int 						iGpsWeek;
	double 					dRoverSecond;
	
		
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);

	
	
	// header
	*q++=0xB5;
	*q++=0x62;
	// class
	*q++=0x01;
	// ID
	*q++=0x04;
	
	// length
	q+=2;
	
	// GPS时 ms
	setU4(q,(unsigned int)(dRoverSecond*1000+0.5)); 					q+=4;
	
	setU2(q,(unsigned short)(g_dDop[0]*100+0.5)); 																q+=2;
	setU2(q,(unsigned short)(g_dDop[1]*100+0.5)); 																q+=2;
	setU2(q,(unsigned short)(g_dDop[4]*100+0.5)); 																q+=2;
	setU2(q,(unsigned short)(g_dDop[3]*100+0.5)); 																q+=2;
	setU2(q,(unsigned short)(g_dDop[2]*100+0.5)); 																q+=2;
	setU2(q,100); 																q+=2;
	setU2(q,100); 																q+=2;
	
	
	n=(int)(q-g_cSendBuf)+2;    
	setU2(g_cSendBuf+4,(unsigned short)(n-8));
	setcs(g_cSendBuf,n);

	g_usSendIndex = n;
	UartSend(huart, g_cSendBuf, n);
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////////发送基准站坐标/////////////////////////////
//////////////////////////////////////////////////////////////////////////  
void SendBasePos( UART_HandleTypeDef *huart )
{	
	unsigned char *q=g_cSendBuf,n;
		
	
	
	// header
	*q++=0xB5;
	*q++=0x62;
	// class
	*q++=0x01;
	// ID
	*q++=0x11;
	// length
	q+=2;
	
	// 已经设置基准站坐标
	if(g_tBasePose.uiUserSetFlag==1)
	{
		setI4(q,(int)(g_tBasePose.dUserSetX*100)); q+=4;
		setI4(q,(int)(g_tBasePose.dUserSetY*100)); q+=4;
		setI4(q,(int)(g_tBasePose.dUserSetZ*100)); q+=4;
	}
	// 滤波获取基准站坐标
	else if(g_tBasePose.uiPosFilterFlag==1)
	{
		setI4(q,(int)(g_tBasePose.dFilterX*100)); q+=4;
		setI4(q,(int)(g_tBasePose.dFilterY*100)); q+=4;
		setI4(q,(int)(g_tBasePose.dFilterZ*100)); q+=4;
	}
	else
	{
		return;
	}
	n=(int)(q-g_cSendBuf)+2;    
	setU2(g_cSendBuf+4,(unsigned short)(n-8));
	setcs(g_cSendBuf,n);

	UartSend(huart, g_cSendBuf, n);
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////////发送基准站坐标/////////////////////////////
//////////////////////////////////////////////////////////////////////////  
void SendBasePosAscii( UART_HandleTypeDef *huart )
{	
		
	if(g_tBasePose.uiUserSetFlag==0)
		return;
	
	g_usSendIndex = 0;
	memset( g_cSendBuf,0,sizeof(g_cSendBuf));

	// 类型标志
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_START);

	// 数据帧头标志 
	AddFieldChar (g_cSendBuf, &g_usSendIndex, "GPPOS");
	AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);

	//  用户设置位置标志	
	sprintf(g_cFieldBuf,"%0d",g_tBasePose.uiUserSetFlag);	
	AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
  AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
	
	// 日期
	
	sprintf(g_cFieldBuf,"%0.3lf,%0.3lf,%0.3lf",g_tBasePose.dUserSetX,g_tBasePose.dUserSetY,g_tBasePose.dUserSetZ);	
	AddFieldChar (g_cSendBuf, &g_usSendIndex, g_cFieldBuf);
  //AddFieldChar (g_cSendBuf, &g_usSendIndex, BD_SENTENCE_COMMA);
			
	// 校验和
	BDAddCheckSum (g_cSendBuf, &g_usSendIndex);
	g_cSendBuf[g_usSendIndex++] 	= 0x0D; 
	g_cSendBuf[g_usSendIndex++] 	= 0x0A; 
	g_cSendBuf[g_usSendIndex] 		= '\0'; 
	
	UartSend(huart, g_cSendBuf, g_usSendIndex);
 
}


void GetDataPos( char *pcInputSentence )
{
  unsigned short usValue;

	//if (g_tBasePose.uiUserSetFlag!=0)
	{
		Field( pcInputSentence,g_cFieldBuf,2 );
		g_tBasePose.dUserSetX = atof( g_cFieldBuf );

		Field( pcInputSentence,g_cFieldBuf,3 );
		g_tBasePose.dUserSetY = atof( g_cFieldBuf );

		Field( pcInputSentence,g_cFieldBuf,4 );
		g_tBasePose.dUserSetZ = atof( g_cFieldBuf );
		
		Field( pcInputSentence,g_cFieldBuf,5 );
		g_tBasePose.uiUserSetFlag = atoi( g_cFieldBuf );
	}
	
	// 保存FLASH标志
	Field( pcInputSentence,g_cFieldBuf,1 );

	g_usFlashProgrammeFlag	= 2;//atoi( g_cFieldBuf );
	
	g_uiFlashProgrammerTic  = HAL_GetTick();
	
		   
}

void GetDataType( char *pcInputSentence )
{
	Field( pcInputSentence,g_cFieldBuf,1 );
	g_tBasePose.uiStationType = atoi( g_cFieldBuf );
	g_usFlashProgrammeFlag	= 1;	
	g_uiFlashProgrammerTic  = HAL_GetTick();		   
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////rover 串口接收 rawdata//////////////////////////
////////////////////////////////////////////////////////////////////////// 
void ProcessRoverRaw( )
{
	unsigned short	i,usData;
	
	
	while (g_usRoverRxCurPos != g_usRoverRxCmdPos)
	{
		
		memcpy(g_tRawRover.buff,g_tUartRover[g_usRoverRxCmdPos].ucRxBuff,sizeof(g_tRawRover.buff));
		g_tRawRover.len 													= g_tUartRover[g_usRoverRxCmdPos].usLength;
		
		//memset(g_tUartRover[g_usRoverRxCmdPos].ucRxBuff,0,sizeof(g_tUartRover[g_usRoverRxCmdPos].ucRxBuff));
		//g_tUartRover[g_usRoverRxCmdPos].usLength 	= 0;
		
		decode_ubx(&g_tRawRover,1);
			
		g_usRoverRxCmdPos++;
		if (g_usRoverRxCmdPos>=UART_RECV_CMD_BUF_CNT)
			g_usRoverRxCmdPos = 0;
		if( (g_usRawRoverFlag==0)&&(g_usUserState==0) )
		{
			
			SleepMode_Measure();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////rover 串口接收 rawdata//////////////////////////
////////////////////////////////////////////////////////////////////////// 
void ProcessBaseRaw( )
{
	unsigned short	i,usData;
	
	
	while (g_usBaseRxCurPos != g_usBaseRxCmdPos)
	{
		memcpy(g_tRawBase.buff,g_tUartBase[g_usBaseRxCmdPos].ucRxBuff,sizeof(g_tRawBase.buff));
		g_tRawBase.len 	= g_tUartBase[g_usBaseRxCmdPos].usLength;
		//if(g_tRawBase.len>20)
		decode_ubx(&g_tRawBase,0);
		
		g_usBaseRxCmdPos++;
		if (g_usBaseRxCmdPos>=UART_RECV_CMD_BUF_CNT)
			g_usBaseRxCmdPos = 0;
		if( (g_usRawRoverFlag==0)&&(g_usUserState==0) )
		{
			SleepMode_Measure();
		}
	}
}