#include "main.h"
#include "usart.h"
#include "global.h"
#include "rtklib.h"
#include "TimeConvert.h"
#include "MyConst.h"


UART_HandleTypeDef Uart1Handle;
UART_HandleTypeDef Uart3Handle;
UART_HandleTypeDef Uart6Handle;
UART_HandleTypeDef Uart7Handle;

void GetDataPos( char *pcInputSentence );
void GetDataType( char *pcInputSentence );
extern uint8_t aTxBuffer[];

UART_FIFO_Typedef_t gFIFO_Uart[4] = {0};
uint8_t gTxBuf[4][TX_BUFFER_SIZE] = {0};
uint8_t gRxBuf[4][RX_BUFFER_SIZE] = {0};
UART_HandleTypeDef Huart[4];
void FIFO_UartInit(int index)
{  	
	Huart[0] = Uart1Handle;//,huart3,huart6,huart7
	Huart[1] = Uart3Handle;
	Huart[2] = Uart6Handle;
	Huart[3] = Uart7Handle;

  FIFO_UartVarInit(&gFIFO_Uart[index],
                     &Huart[index],
                     gTxBuf[index],
                     gTxBuf[index],
                     TX_BUFFER_SIZE,
                     RX_BUFFER_SIZE,
                     NULL,
                     NULL,
                     NULL);
}

void FIFO_UbloxUartIRQ(int index)
{
    FIFO_UartIRQ(&gFIFO_Uart[index]);
}
//////////////////////////////////////////////////////////////////////////
////////////////////////////////初始化串口////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void UartInit(UART_HandleTypeDef *pUartHandle,unsigned short usUartFlag,unsigned int uiBaudRate)
{
	
	if(usUartFlag==1)
	{
		pUartHandle->Instance        = USART1;
		pUartHandle->Init.BaudRate   = uiBaudRate;
	}
	else if(usUartFlag==2)
	{
		pUartHandle->Instance        = USART2;
		pUartHandle->Init.BaudRate   = uiBaudRate;
	}
	else if(usUartFlag==3)
	{
		pUartHandle->Instance        = USART3;
		pUartHandle->Init.BaudRate   = uiBaudRate;
	}
	else if(usUartFlag==6)
	{
		pUartHandle->Instance        = USART6;
		pUartHandle->Init.BaudRate   = uiBaudRate;
	}
	else if(usUartFlag==7)
	{
		pUartHandle->Instance        = UART7;
		pUartHandle->Init.BaudRate   = uiBaudRate;
	}

  
  pUartHandle->Init.WordLength = UART_WORDLENGTH_8B;
  pUartHandle->Init.StopBits   = UART_STOPBITS_1;
  pUartHandle->Init.Parity     = UART_PARITY_NONE;
  pUartHandle->Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  pUartHandle->Init.Mode       = UART_MODE_TX_RX;

  if (HAL_UART_DeInit(pUartHandle) != HAL_OK)
	{
	
	}
	if (HAL_UART_Init(pUartHandle) != HAL_OK)
  {
    /* Initialization Error */
    //Error_Handler();
  } 
	
	//__HAL_UART_ENABLE_IT(pUartHandle, UART_IT_TXE);
	//if(115200 == uiBaudRate)
	{
		__HAL_UART_ENABLE_IT(pUartHandle, UART_IT_RXNE);
	}
} 

//////////////////////////////////////////////////////////////////////////
//////////////////////////////写串口发送寄存器////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void UsartSendData(UART_HandleTypeDef *pUartHandle, unsigned short usData)
{
	/* Transmit Data */
	
	//if(HAL_UART_Transmit_IT(pUartHandle, &ucData, 1)!= HAL_OK)
  {
    //Error_Handler();
  }
	
	//转发串口是否已经发送完毕
	while (__HAL_UART_GET_IT(pUartHandle, UART_IT_TXE) == RESET)
	{

	}		
		
  pUartHandle->Instance->TDR = (uint8_t)usData;		
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////串口接收////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void UartReceive( UART_HandleTypeDef *ptUart )
{
	unsigned char ucData;
	unsigned short usMask = ptUart->Mask;
	
	if((__HAL_UART_GET_IT(ptUart, UART_IT_RXNE) != RESET) && (__HAL_UART_GET_IT_SOURCE(ptUart, UART_IT_RXNE) != RESET))
	{
		ucData = (uint16_t)(ptUart->Instance->RDR & usMask);		
	}
}
void UartRecvIntCmd( UART_HandleTypeDef *ptUart  )
{

	unsigned short usData;
	unsigned short usMask = ptUart->Mask;
	unsigned int oldOutputType = g_tBasePose.uiOutputType;
    	
	if((__HAL_UART_GET_IT(ptUart, UART_IT_RXNE) != RESET))
	{
		
		g_usUserState = 1;
		usData = (uint16_t)(ptUart->Instance->RDR);	

		g_cRecvBufUart2[g_usRecvCntUart2] = usData;
		
		if ( (g_cRecvBufUart2[g_usRecvCntUart2 - 1] == 0x0d) && (g_cRecvBufUart2[g_usRecvCntUart2] == 0x0a))
		{
					
			if ((g_cRecvBufUart2[3] == 'P') && (g_cRecvBufUart2[4] == 'O') && (g_cRecvBufUart2[5] == 'S'))
			{
				GetDataPos( g_cRecvBufUart2);
			}
			else if ((g_cRecvBufUart2[3] == 'T') && (g_cRecvBufUart2[4] == 'Y') && (g_cRecvBufUart2[5] == 'P')&& (g_cRecvBufUart2[6] == 'E'))
			{
				GetDataType(g_cRecvBufUart2);
			}
			else if ((g_cRecvBufUart2[1] == 'N') && (g_cRecvBufUart2[2] == 'M') && (g_cRecvBufUart2[3] == 'E') && (g_cRecvBufUart2[4] == 'A'))
			{
					g_tBasePose.uiOutputType = NMEA;
			}
			else if ((g_cRecvBufUart2[1] == 'P') && (g_cRecvBufUart2[2] == 'I') && (g_cRecvBufUart2[3] == 'X'))
			{
					g_tBasePose.uiOutputType = PIX;
			}
			else if ((g_cRecvBufUart2[1] == 'J') && (g_cRecvBufUart2[2] == 'I') && (g_cRecvBufUart2[3] == 'Y') && (g_cRecvBufUart2[4] == 'I'))
			{
					g_tBasePose.uiOutputType = JIYI;
			}
			else if ((g_cRecvBufUart2[1] == '0') && (g_cRecvBufUart2[2] == 'T') && (g_cRecvBufUart2[3] == 'E') && (g_cRecvBufUart2[4] == 'C') && (g_cRecvBufUart2[5] == 'H'))
			{
					g_tBasePose.uiOutputType = ZEROTECH;
			}
			
		if(g_tBasePose.uiOutputType != oldOutputType)
		{
				g_usFlashProgrammeFlag	= 2;	
				g_uiFlashProgrammerTic  = HAL_GetTick();
				if(g_ucStationType == 1)
				{
					switch (g_tBasePose.uiOutputType)
				{
				case NMEA:
				case PIX:
					UartInit(&Uart1Handle,1,115200);
					UartInit(&Uart6Handle,6,115200);
					break;
				case JIYI:
				case ZEROTECH:
					UartInit(&Uart1Handle,1,38400);
					UartInit(&Uart6Handle,6,38400);
					break;
				default:
					break;
				}
			}
		}				
			g_usRecvFlagUart2 		= 1;
			g_usRecvCntUart2			= 0;
			memset(g_cRecvBufUart2,0,sizeof(g_cRecvBufUart2));
		}
		else
		{
			g_usRecvCntUart2++;

				if(g_usRecvCntUart2 == 1)
				{
						if((unsigned char)g_cRecvBufUart2[0] != '$')				
						{
								g_usRecvCntUart2 = 0;
						}
				}

			if(g_usRecvCntUart2 >= UART_RECV_BUF_LENTH)
			{
					g_usRecvCntUart2 				= 0;
					memset(g_cRecvBufUart2,0,sizeof(g_cRecvBufUart2));
			}
		}
	}
	else 
	{
		HAL_UART_IRQHandler(ptUart);
	}
	
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////串口转发////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
//unsigned char ucData;
unsigned short usData;
void UartReceiveAndSend( UART_HandleTypeDef *ptUartSrc ,UART_HandleTypeDef *ptUartDst )
{
	
	unsigned short usMask = ptUartSrc->Mask;
	
	if((__HAL_UART_GET_IT(ptUartSrc, UART_IT_RXNE) != RESET))
	{
		usData = (uint16_t)(ptUartSrc->Instance->RDR);
		
		//转发串口是否已经发送完毕
	while (__HAL_UART_GET_IT(ptUartDst, UART_IT_TXE) == RESET)
	{

	}					
		UsartSendData(ptUartDst, usData);
	}	
		
	//UsartSendData(ptUartDst, usData);

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////串口转发////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void UartBaseStation( UART_HandleTypeDef *ptUartSrc ,UART_HandleTypeDef *ptUartDst,UART_HandleTypeDef *ptUartDst2,UART_HandleTypeDef *ptUartDst3 )
{
	
	unsigned short usMask = ptUartSrc->Mask;
	
	if(g_ucStationType==0)
	{
		if  ( g_ucPpsSign == 1 )
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);	
		}
		else
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);	
		}	
	}
	
	if((__HAL_UART_GET_IT(ptUartSrc, UART_IT_RXNE) != RESET))
	{
		usData = (uint16_t)(ptUartSrc->Instance->RDR);
		g_usRoverRevFlag = input_ubx(&g_tRawRevRover,usData,1);
				
		UsartSendData(ptUartDst, usData);
		UsartSendData(ptUartDst2, usData);
		
		//转发串口是否已经发送完毕
	while (__HAL_UART_GET_IT(ptUartDst, UART_IT_TXE) == RESET)
	{

	}		
		//转发串口是否已经发送完毕
	while (__HAL_UART_GET_IT(ptUartDst2, UART_IT_TXE) == RESET)
	{

	}	
	
		//转发串口是否已经发送完毕
	//while (__HAL_UART_GET_IT(ptUartDst3, UART_IT_TXE) == RESET)
	{

	}	
		
	}	
		
	//UsartSendData(ptUartDst, usData);

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////rover 串口接收 rawdata//////////////////////////
////////////////////////////////////////////////////////////////////////// 
void ReceiveRoverRaw( UART_HandleTypeDef *huart )
{
	unsigned short usData;
	

		//if((__HAL_UART_GET_IT(huart, UART_IT_RXNE) != RESET) )
		while((__HAL_UART_GET_IT(huart, UART_IT_RXNE) == RESET))		
		{
		
		}
		
		usData = (uint16_t)(huart->Instance->RDR);
		g_usRoverRevFlag = input_ubx(&g_tRawRevRover,usData,1);
		

		
	
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////base 串口接收 rawdata///////////////////////////
//////////////////////////////////////////////////////////////////////////
void ReceiveBaseRaw( UART_HandleTypeDef *huart )
{
	unsigned short usData;;

		if((__HAL_UART_GET_IT(huart, UART_IT_RXNE) != RESET))
		{
			usData = (uint16_t)(huart->Instance->RDR);;
			g_usBaseRevFlag = input_ubx(&g_tRawRevBase,usData,0);	
		}			
		else
		{
			HAL_UART_IRQHandler(huart);
		}
}



//////////////////////////////////////////////////////////////////////////
////////////////////////////////提取位置设置//////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void UartCmdPos( char *pcInputData)
{
	unsigned short usValue;
	
	Field(pcInputData,g_cFieldBuf,1);
	usValue = atoi(g_cFieldBuf);
	
	Field(pcInputData,g_cFieldBuf,2);
	usValue = atoi(g_cFieldBuf);
	
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////处理串口接收数据////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void ProcessUartRecv()
{	
	
    while(g_usRecvCmdSetPosUart2 != g_usRecvCmdCurPosUart2)
    {
        memcpy(g_cCmdBuf,&g_cRecvCacheUart2[g_usRecvCmdCurPosUart2][0],UART_RECV_BUF_LENTH);

        g_usRecvCmdCurPosUart2++;
				if(g_usRecvCmdCurPosUart2 >= UART_RECV_CMD_BUF_CNT)
				{
					g_usRecvCmdCurPosUart2 = 0; 
				}       
        
        if (g_cCmdBuf[0] != '$') 
        {      
            return;
				}	
				else if ((g_cCmdBuf[3] == 'P') && (g_cCmdBuf[4] == 'O') && (g_cCmdBuf[5] == 'S'))
				{
					GetDataPos( g_cCmdBuf);
				}
				
    }
	
}




//////////////////////////////////////////////////////////////////////////
////////////////////////////////初始化GPIO//////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void GpioInit( void )
{
	GPIO_InitTypeDef  gpio_init_structure;
#if 0
  //配置GPIOA
	/* Enable the GPIOA clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* Configure the GPIO_A11  GPIO_A12*/
	gpio_init_structure.Pin = GPIO_PIN_11|GPIO_PIN_12;
	gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_init_structure.Pull = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(GPIOA, &gpio_init_structure);

	/* By default, turn off GPIO_A11  GPIO_A12 */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_SET);
	
	//配置GPIOI
	/* Enable the GPIOI clock */
	__HAL_RCC_GPIOI_CLK_ENABLE();
	
	/* Configure the GPIO_I1  GPIO_I2 pin */
	gpio_init_structure.Pin = GPIO_PIN_1|GPIO_PIN_2;
	gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_init_structure.Pull = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(GPIOI, &gpio_init_structure);

	/* By default, turn off GPIO_I1  GPIO_I2  */
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);	
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);	
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1, GPIO_PIN_RESET);	
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_2, GPIO_PIN_RESET);
#else
  //配置GPIOA
	/* Enable the GPIOA clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	/* Configure the GPIO_A11*/
	gpio_init_structure.Pin = GPIO_PIN_11;
	gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_init_structure.Pull = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(GPIOA, &gpio_init_structure);

	/* By default, turn off GPIO_A11 */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
	
	//配置GPIOD
	/* Enable the GPIOD clock */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	
	/* Configure the GPIO_I1  GPIO_I2 pin */
	gpio_init_structure.Pin = GPIO_PIN_13;
	gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_init_structure.Pull = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(GPIOD, &gpio_init_structure);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);	
  	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
#endif	

	//配置GPIOC
	/* Enable the GPIOC clock */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	/* Configure the GPIO_C13  pin */
	gpio_init_structure.Pin = GPIO_PIN_13;
	gpio_init_structure.Mode = GPIO_MODE_IT_RISING;
	gpio_init_structure.Pull = GPIO_NOPULL;
	gpio_init_structure.Speed = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(GPIOC, &gpio_init_structure);	
	
	/* Enable and set EXTI lines 15 to 10 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}



