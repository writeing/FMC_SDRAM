#include "imain.h"
#include "iUblox.h"
#include "usart.h"
#include "FiFOuart.h"
#include "imain.h"
#include "rtklib.h"
#include "rtk.h"
#include "usart.h"
#include "MyConst.h"
#include "global.h"
U8 gInitDoneFlag=1;
U8 uartBuffer6[UART_BUFFER_SIZE]={0};

U8 RtkUart_1_ReadHandler()
{
	int length=0;
	int result;
	//length = FIFO_UartReadBuffer(&gFIFO_Uart[HUART1],uartBuffer1,sizeof(uartBuffer1));
	if(g_tBasePose.uiStationType == 1)		
	{	
		U8 ch = 0;
		while(FIFO_UartReadByte(&gFIFO_Uart[HUART1],&ch) == HAL_OK)
		{
			result = input_ubx(&g_tRawBase,ch,0);
			
		}			
		if(g_usRawRoverFlag == 0 )
		{
			processRtk();
		}//g_usRawRoverFlag
	}
	
	return length;
}

U8 RtkUart_UBLOX_ReadHandler()
{
	kal_int32 length=0;	
  int result;
	//memset(uartBuffer7,0,sizeof(uartBuffer7));	

	
	if (gInitDoneFlag)
	{
		int i = 0;
		
		U8 ch = 0;
		while(FIFO_UartReadByte(&gFIFO_Uart[UBLOX],&ch) == HAL_OK)
				{
					result = input_ubx(&g_tRawRover, ch,1);
				}		
			
			if(g_tBasePose.uiStationType == 1)
			{
				if(g_usRawRoverFlag == 1 )
				{		
					
					processRtk();
				}
			}
		}


	return length;
}
U8 RtkUart_USB_ReadHandler()
{
	kal_int32 length=0;
//	int result;	
	memset(uartBuffer6,0,sizeof(uartBuffer6));

	//length = UartReadBuffer(&gUartDatPort_usb, uartBuffer6, sizeof(uartBuffer6));			
	length = FIFO_UartReadBuffer(&gFIFO_Uart[DATA],uartBuffer6,sizeof(uartBuffer6));
	{
		int i;
		for (i = 0; i < length; i++)
		{
			if (uartBuffer6[i] == '$')
			{
				//ReadRTKSetCmd(uartBuffer6 + i, length - i);								
			}
		}
	}		
	return length;
}
void icegpsMain()
{	

//	ggt_mem_init();
//	VerifyKeyInInit();
	RtkMain();
	
	UtcParaInit();

	gInitDoneFlag=1;
//#if 1
//    RtkUart_2_Init(UART_BAUD_9600);	
	g_tBasePose.uiStationType = 0;// 强制设置为流动站   0 :jizhan   1 : liudongzhan
	g_ucStationType 						= 0;// 强制设置为流动站   0 :jizhan   1 : liudongzhan
	SetUblox();	
	UartSetBaudRate(&Huart[UBLOX],115200);
    g_ucStationType 						= g_tBasePose.uiStationType;
	//g_tBasePose.uiUserSetFlag  	= 0;  //上电默认为用户未设置基准站坐标
	g_tBasePose.uiPosFilterFlag = 0;  //上电默认基准站滤波坐标无效
	g_tBasePose.dFilterX 				= 0.0;
	g_tBasePose.dFilterY 				= 0.0;
	g_tBasePose.dFilterZ 				= 0.0;
	
	

	g_usInitCompelet = 1;
	

}
/**
	* @brief  Process RtkWork Send Result to Uart
	* @param  None
	* @retval None
 */
void processRtk(void)
{
	uint32_t uiRtkTic = 0,uiSendIndx=0;
	
	if  ( fabs(HAL_GetTick()-g_uiPpsTic)<200 )
	{
		// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);	
	}
	else 
	{							
		if  ( fabs(HAL_GetTick()-g_uiPpsTic)>900 )
		{
			g_usBaseRevFlag 	= 0;
			g_usRoverRevFlag 	= 0;
		}
		// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);						
	}
	
	//if ( HAL_GetTick()-g_uiPpsTic <300)
	//	return;
	
	//if (g_usPpsInt==0)
	//	return;
	
	//g_usPpsInt = 0;
	
	if (g_usRawRoverFlag==0)
		return;

	uiRtkTic   = HAL_GetTick();
	
	//g_tRtk.opt.elmin = 10*D2R;
	
	g_usRtkProcessFlag = 1;
	
	CalcBaseLineWrok( );	
	//SendRoverView(&Uart6Handle,HAL_GetTick()-uiRtkTic);
	//SendDataRtk(&Uart6Handle,HAL_GetTick()-uiRtkTic);
		//SendRTKUbxData(&Uart6Handle);
		//memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;	
	#if 0
	if (g_tBasePose.uiOutputType == NMEA)
	{
		SendDataGGA(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;
			
		SendDataVTG(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;
		
		SendDataRtk(&Uart6Handle,HAL_GetTick()-uiRtkTic);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;
		
	}
	else if(g_tBasePose.uiOutputType == PIX)
	{
		if (MegHEARTBEAT(&Uart6Handle))
		{
			memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
			uiSendIndx	+= g_usSendIndex;
		}
			
		MegGpsTime(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;
		
		MegVelENU(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;
		
		MegPosLLH(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;
		
		MegDops(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;
	}
	else if(g_tBasePose.uiOutputType == JIYI)
	{
		SendDataSOL(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;	
		
		SendDataPOSLLH(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;
		
		SendDataSTATUS(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;
		
		SendDataVELNED(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;
		
		/*SendDataDOP(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;*/
		
	}	
	else if(g_tBasePose.uiOutputType == ZEROTECH)
	{
		SendDataSOL(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;	
		
		SendDataPOSLLH(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;		
		
		SendDataVELNED(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;
		
		SendDataTIMEUTC(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;

		/*SendDataDOP(&Uart6Handle);
		memcpy(&g_ucSendBuf[uiSendIndx],g_cSendBuf,g_usSendIndex);
		uiSendIndx	+= g_usSendIndex;*/
		
	}	
	#endif
	// StartDMA(&Uart2Handle,uiSendIndx);
	//StartDMA(&Uart6Handle,uiSendIndx);
	
		// GPIO指示
	// ublox PPS				<---->PC13
	// PPS  						<---->PA12
	// ROVER 						<---->PA11   FIX
	// BASE  						<---->PI2    FLOAT
	// FIX							<---->PI1    NONE
	
	if(g_tRtk.sol.stat == SOLQ_NONE)
	{
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
	}
	else if(g_tRtk.sol.stat == SOLQ_FIX)
	{	
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_13));
	}

	// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11));
	//g_tRtk.opt.elmin 		= 10*D2R;
	g_usRawRoverFlag 		= 0;
	g_usRtkProcessFlag 	= 0;
			
}
void ProcessUartTask(void)
{
//	if( g_usPpsInt !=1 )
//		return;
//	
//	if( fabs(HAL_GetTick()-g_uiPpsTic)<10 )
//		return;
//	
//	g_usPpsInt	= 0;
	
	//SendBasePos(&Uart1Handle);	
	// SendBasePos(&Uart2Handle);	
	printf("send base pos\r\n");
	SendBasePos(&Uart6Handle);	
	
}



// 检查基准站设置坐标是否偏离当前真实坐标
void CheckBasePosValid()
{
	double 					dBasePosDiff = 0.0;
	unsigned short 	usBasePosDiffFlag = 0;
	
	if( (g_tBasePose.dFilterX !=0.0)&&(g_tBasePose.dFilterY !=0.0)&&(g_tBasePose.dFilterZ !=0.0) )
	{
		dBasePosDiff  += (g_tBasePose.dUserSetX - g_tBasePose.dFilterX)*(g_tBasePose.dUserSetX - g_tBasePose.dFilterX);
		dBasePosDiff  += (g_tBasePose.dUserSetY - g_tBasePose.dFilterY)*(g_tBasePose.dUserSetY - g_tBasePose.dFilterY);
		dBasePosDiff  += (g_tBasePose.dUserSetZ - g_tBasePose.dFilterZ)*(g_tBasePose.dUserSetZ - g_tBasePose.dFilterZ);
		dBasePosDiff  =  sqrt(dBasePosDiff);
		
		if(dBasePosDiff>20.0)
		{
			usBasePosDiffFlag = 1; //偏差超门限
		}
	}
	if( 1== usBasePosDiffFlag )
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);	
		HAL_Delay(40);	
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);	
		HAL_Delay(40);		
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);	
		HAL_Delay(40);	
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);	
	
		
	}

}


