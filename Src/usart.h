#ifndef __USART_H
#define __USART_H

#include "main.h"
#include "FiFOuart.h"

#define TX_BUFFER_SIZE 1024
#define RX_BUFFER_SIZE 1024


typedef enum 
{
	HUART1 = 0,
	HUART3 = 1,
	HUART6 = 2,
	HUART7 = 3
}HART_INDEX;

typedef enum 
{
	FTDI = 0,
	CMD = 1,
	DATA = 2,
	UBLOX = 3
}HART_AS_NAME;




extern UART_HandleTypeDef Huart[4];

extern UART_FIFO_Typedef_t gFIFO_Uart[4];
extern uint8_t gTxBuf[4][TX_BUFFER_SIZE];
extern uint8_t gRxBuf[4][RX_BUFFER_SIZE];

void FIFO_UartInit(int index);
void FIFO_UbloxUartIRQ(int index);


#define BD_SENTENCE_COMMA ","
#define BD_SENTENCE_ASTERISK "*"
#define BD_SENTENCE_START "$"

//extern void UartDMAInit(UART_HandleTypeDef *pUartHandle,unsigned short usUartFlag);
extern void UartInit( UART_HandleTypeDef *huart,unsigned short usUartFlag,unsigned int uiBaudRate );
extern void UartReceive( UART_HandleTypeDef *ptUart );
extern void UartReceiveAndSend( UART_HandleTypeDef *ptUartSrc ,UART_HandleTypeDef *ptUartDst );
extern void UartBaseStation( UART_HandleTypeDef *ptUartSrc ,UART_HandleTypeDef *ptUartDst,UART_HandleTypeDef *ptUartDst2,UART_HandleTypeDef *ptUartDst3 );
extern void ReceiveRoverRaw( UART_HandleTypeDef *huart  );
extern void ReceiveBaseRaw( UART_HandleTypeDef *huart  );
extern void MY_UART_IRQHandler(UART_HandleTypeDef *huart);
extern void UartSend(UART_HandleTypeDef *pUartHandle, char *pcData, unsigned short usLength);
extern void SendDataRtk( UART_HandleTypeDef *huart,unsigned int uiRtkTic  );
extern void SendDataGGA( UART_HandleTypeDef *huart );
extern void SendDataZDA( UART_HandleTypeDef *huart );
extern void SendDataVTG( UART_HandleTypeDef *huart );
extern void SendRTKUbxData(UART_HandleTypeDef *huart);
extern void SendRoverView(UART_HandleTypeDef *huart,unsigned int uiRtkTic);
extern void SendDataPOSLLH( UART_HandleTypeDef *huart );
extern void SendDataSOL( UART_HandleTypeDef *huart );
extern void SendDataSTATUS( UART_HandleTypeDef *huart );
extern void SendDataVELNED( UART_HandleTypeDef *huart );
extern void SendDataTIMEUTC( UART_HandleTypeDef *huart );
extern void SendDataDOP( UART_HandleTypeDef *huart );
extern void SendBasePos( UART_HandleTypeDef *huart );
extern void SendBasePosAscii( UART_HandleTypeDef *huart );
extern void ProcessUartRecv();
extern void UsartSendData(UART_HandleTypeDef *pUartHandle, unsigned short usData);
void ProcessRoverRaw();
void ProcessBaseRaw( );
extern UART_HandleTypeDef Uart1Handle;
extern UART_HandleTypeDef Uart7Handle;
extern UART_HandleTypeDef Uart3Handle;
extern UART_HandleTypeDef Uart6Handle;
extern void GpioInit( void );

#endif
