// 伪距差分
#ifndef _GLOBAL_H
#define _GLOBAL_H
#include "struct.h"
#include "TimeConvert.h"
#include "rtklib.h"
#include "MyConst.h"

extern unsigned char g_ucDirectMode;
extern raw_t g_tRawBase;
extern raw_t g_tRawRover; 
extern rtk_t g_tRtk;
extern obs_t obs;
extern obsd_t data[MAXOBS*1];
extern obsd_t dataBase[MAXOBS*1];
extern nav_t nav;
extern eph_t eph[MAXSAT*1];
extern eph_t ephBase[MAXSAT*1];
extern double					g_dElev[MAXSAT];
extern TSATELEVSTRUCT 	g_tSatElev;

extern raw_t				g_tRawRevBase;
extern raw_t				g_tRawRevRover;


extern unsigned short g_usSlipFlag ;
extern unsigned short g_usCommonSat[MAXSAT];
extern unsigned short g_usSatNum[10];
extern unsigned short g_usRtkProcessFlag;
extern double					g_dDop[5];

// 定向相关

extern TDIFFOBSINFOSTRUCT			g_tDiffObsInfo;								// 差分信息
extern TDIFFOBSINFOSTRUCT			g_tRevBaseInfo;								// 接收到的基准站信息
extern unsigned short					g_usObsErrFlag;
extern unsigned short					g_usRtkRatio;
extern unsigned short 				g_usRawBaseFlag;
extern unsigned short 				g_usRawRoverFlag;
extern unsigned short 				g_usBaseRevFlag;
extern unsigned short 				g_usRoverRevFlag;

// 用户参数设置
extern TBASEPOSSTRUCT					g_tBasePose;
extern TBASEPOSSTRUCT					g_tReadPosFromBase;
extern TUARTRMOSTRUCT					g_tUartRmo;
extern unsigned char					g_ucDataBuffer[1000];
extern unsigned short					g_usFlashProgrammeFlag;
extern unsigned int						g_uiFlashProgrammerTic ;
extern unsigned short 				g_usUserState;
extern unsigned char					g_ucStationType;
extern unsigned short					g_usInitCompelet;


// GNSS模块相关
extern unsigned short					g_usPpsInt;
extern unsigned int						g_uiPpsTic;
extern unsigned char					g_ucPpsSign ;

// 时间相关
extern TBDUTCTIMEPARASTRUCT 	g_tBdUtcTimePara;               // BD UTC结构体
extern TGPSUTCTIMEPARASTRUCT 	g_tGpsUtcTimePara;							// GPS UTC结构体

extern TUARTRXCACHESTRUCT			g_tUartRover[UART_RECV_CMD_BUF_CNT];	
extern unsigned short					g_usRoverRxCurPos;
extern unsigned short					g_usRoverRxCmdPos;
extern TUARTRXCACHESTRUCT			g_tUartBase[UART_RECV_CMD_BUF_CNT];
extern unsigned short					g_usBaseRxCurPos;
extern unsigned short					g_usBaseRxCmdPos;


extern char 											g_cSendBufUart0[UART_SEND_BUF_LENTH];         											// 串口0发送缓存
extern volatile unsigned short 		g_usSendSetPosUart0 ;
extern volatile unsigned short 		g_usSendCurPosUart0;
extern char 											g_cFieldBuf[UART_RECV_FIELD_LENTH];
extern char 											g_cSendBuf[UART_SEND_BUF_LENTH];
extern unsigned char 							g_ucSendBuf[UART_SEND_BUF_LENTH];
extern unsigned short 						g_usSendIndex; 

extern char 											g_cSendBufUart1[UART_SEND_BUF_LENTH];          											// 串口1发送缓存
extern volatile unsigned short 		g_usSendSetPosUart1;
extern volatile unsigned short 		g_usSendCurPosUart1;  

extern char 											g_cSendBufUart2[UART_SEND_BUF_LENTH];          											// 串口1发送缓存
extern volatile unsigned short 		g_usSendSetPosUart2;
extern volatile unsigned short 		g_usSendCurPosUart2;  

extern char 											g_cRecvCacheUart2[UART_RECV_CMD_BUF_CNT][UART_RECV_BUF_LENTH];      // 串口接收CACHE
extern char 											g_cRecvBufUart2[UART_RECV_BUF_LENTH];																// 串口接收BUFF
extern char 											g_cCmdBuf[UART_RECV_BUF_LENTH];       															// 串口指令接收缓存
extern unsigned char   						g_ucCmdBuf[UART_RECV_BUF_LENTH];
extern unsigned short							g_usRecvFlagUart2;
extern volatile unsigned short		g_usRecvCntUart2; 																									// 接收数据计数
extern volatile unsigned short 		g_usRecvCmdSetPosUart2;      																				// 串口保存数据缓存位置      
extern volatile unsigned short 		g_usRecvCmdCurPosUart2;																							//串口读取数据缓存位置

extern double	 dTxBuffer[BUFFER_SIZE];
extern double	dRxBuffer[BUFFER_SIZE];

#endif


