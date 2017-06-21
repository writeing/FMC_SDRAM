#include "global.h"
#include "struct.h"
#include "rtklib.h"
#include "TimeConvert.h"
#include "MyConst.h"

unsigned char		g_ucDirectMode = 0;
// RTK相关
raw_t 					g_tRawBase 									__attribute__ ((section ("EXRAM")));
raw_t 					g_tRawRover 								__attribute__ ((section ("EXRAM")));
rtk_t 					g_tRtk 											__attribute__ ((section ("EXRAM")));
obs_t 					obs 												__attribute__ ((section ("EXRAM")));
obsd_t 					data[MAXOBS*1] 							__attribute__ ((section ("EXRAM")));
obsd_t 					dataBase[MAXOBS*1] 					__attribute__ ((section ("EXRAM")));
nav_t 					nav 												__attribute__ ((section ("EXRAM")));
eph_t 					eph[MAXSAT*1] 							__attribute__ ((section ("EXRAM")));
eph_t 					ephBase[MAXSAT*1] 					__attribute__ ((section ("EXRAM")));
double					g_dElev[MAXSAT]							__attribute__ ((section ("EXRAM")));
TSATELEVSTRUCT 	g_tSatElev									__attribute__ ((section ("EXRAM")));

raw_t				g_tRawRevBase;
raw_t				g_tRawRevRover;

unsigned short 	g_usSlipFlag = 0;
unsigned short 	g_usCommonSat[MAXSAT];
unsigned short 	g_usSatNum[10] = {0};
unsigned short 	g_usRtkProcessFlag = 0;
double					g_dDop[5];


// 定向相关

TDIFFOBSINFOSTRUCT			g_tDiffObsInfo 		__attribute__ ((section ("EXRAM")));								// 差分信息
TDIFFOBSINFOSTRUCT			g_tRevBaseInfo 		__attribute__ ((section ("EXRAM")));								// 接收到的基准站信息
unsigned short					g_usObsErrFlag;																												// 观测量误差超限
unsigned short					g_usRtkRatio = 0;																											
unsigned short 					g_usRawBaseFlag = 0;																									// 串口中断接收基准站观测量解析完毕的标志
unsigned short 					g_usRawRoverFlag = 0;																									// 串口中断接收流动站观测量解析完毕的标志
unsigned short 					g_usBaseRevFlag = 0;																									// 串口中断接收基准站观测量数据保存完毕的标志
unsigned short 					g_usRoverRevFlag = 0;																									// 串口中断接收流动站观测量数据保存完毕的标志	

// 用户参数设置
TBASEPOSSTRUCT					g_tBasePose;
TBASEPOSSTRUCT					g_tReadPosFromBase;
TUARTRMOSTRUCT					g_tUartRmo;
unsigned char						g_ucDataBuffer[1000];
unsigned short					g_usFlashProgrammeFlag = 0; //1：切换模式，2：设置位置
unsigned int						g_uiFlashProgrammerTic = 0;
unsigned short 					g_usUserState = 0;                                                   	//0:空闲，1：正在接收外部参数设置
unsigned char						g_ucStationType = 0;																									//0:Base,	1:Rover
unsigned short					g_usInitCompelet = 0;

//GNSS模块相关
unsigned short					g_usPpsInt = 0; //pps中断标志
unsigned int						g_uiPpsTic = 0; //pps中断时的TIC计数
unsigned char						g_ucPpsSign = 0;

// 时间相关
TBDUTCTIMEPARASTRUCT 		g_tBdUtcTimePara;               // BD UTC结构体
TGPSUTCTIMEPARASTRUCT 	g_tGpsUtcTimePara;							// GPS UTC结构体



// 串口相关
TUARTRXCACHESTRUCT				g_tUartRover[UART_RECV_CMD_BUF_CNT]											__attribute__ ((section ("EXRAM"))) ;
unsigned short						g_usRoverRxCurPos = 0;	
unsigned short						g_usRoverRxCmdPos = 0;
TUARTRXCACHESTRUCT				g_tUartBase[UART_RECV_CMD_BUF_CNT] 											__attribute__ ((section ("EXRAM"))) ;
unsigned short						g_usBaseRxCurPos = 0;
unsigned short						g_usBaseRxCmdPos = 0;


char 											g_cSendBufUart0[UART_SEND_BUF_LENTH] 										__attribute__ ((section ("EXRAM"))) = {0};          						// 串口0发送缓存 		
volatile unsigned short 	g_usSendSetPosUart0 = 0;	
volatile unsigned short 	g_usSendCurPosUart0 = 0; 
char 											g_cFieldBuf[UART_RECV_FIELD_LENTH];// 											__attribute__ ((section ("EXRAM")));
char 											g_cSendBuf[UART_SEND_BUF_LENTH];// 												__attribute__ ((section ("EXRAM")));
unsigned char 						g_ucSendBuf[UART_SEND_BUF_LENTH]																				__attribute__ ((section ("EXRAM")));
unsigned short 						g_usSendIndex = 0; 

double 										dTxBuffer[BUFFER_SIZE] 																		__attribute__ ((section ("EXRAM")));
double 										dRxBuffer[BUFFER_SIZE] 																		__attribute__ ((section ("EXRAM")));


char 											g_cSendBufUart1[UART_SEND_BUF_LENTH] 										__attribute__ ((section ("EXRAM"))) = {0};          						// 串口1发送缓存
volatile unsigned short 	g_usSendSetPosUart1 = 0;
volatile unsigned short 	g_usSendCurPosUart1 = 0; 


char 											g_cSendBufUart2[UART_SEND_BUF_LENTH] 										__attribute__ ((section ("EXRAM")))= {0};          						// 串口1发送缓存
volatile unsigned short 	g_usSendSetPosUart2 = 0;
volatile unsigned short 	g_usSendCurPosUart2 = 0;  

char 						g_cRecvCacheUart2[UART_RECV_CMD_BUF_CNT][UART_RECV_BUF_LENTH] 		__attribute__ ((section ("EXRAM")))= {0};      // 串口接收CACHE
char 						g_cRecvBufUart2[UART_RECV_BUF_LENTH];// 															__attribute__ ((section ("EXRAM")))= {0};																// 串口接收BUFF
char 						g_cCmdBuf[UART_RECV_BUF_LENTH];//																		__attribute__ ((section ("EXRAM")));       																		// 串口指令接收缓存
unsigned char   g_ucCmdBuf[UART_RECV_BUF_LENTH];
unsigned short						g_usRecvFlagUart2 = 0;
volatile unsigned short		g_usRecvCntUart2 = 0; 																					// 接收数据计数
volatile unsigned short 	g_usRecvCmdSetPosUart2 = 0;      																// 串口保存数据缓存位置      
volatile unsigned short 	g_usRecvCmdCurPosUart2 = 0;																			//串口读取数据缓存位置



void InitGlobal( void )
{
	
	// 定向相关
	memset( &g_tDiffObsInfo,0,sizeof(g_tDiffObsInfo) );
	memset( &g_tRevBaseInfo,0,sizeof(g_tRevBaseInfo) );
	
	// 用户参数设置
	memset( &g_tBasePose,0,sizeof(g_tBasePose) );
	memset( &g_tReadPosFromBase,0,sizeof(g_tReadPosFromBase) );
	

		
	// 串口相关		
	
	
	memset( g_tUartRover,0,sizeof(g_tUartRover) );
	memset( g_tUartBase,0,sizeof(g_tUartBase) );

	memset( g_cSendBufUart0,0,sizeof(g_cSendBufUart0) );
	g_usSendSetPosUart0 			= 0;
	g_usSendCurPosUart0 			= 0;
	memset( g_cFieldBuf,0,sizeof(g_cFieldBuf) );
  memset( g_cSendBuf,0,sizeof(g_cSendBuf) );
	g_usSendIndex 					= 0;	
	
	
	memset( g_cSendBufUart1,0,sizeof(g_cSendBufUart1) );
	g_usSendSetPosUart1 			= 0;
	g_usSendCurPosUart1 			= 0;

	
	memset( g_cSendBufUart2,0,sizeof(g_cSendBufUart2) );
	g_usSendSetPosUart2 			= 0;
	g_usSendCurPosUart2 			= 0;
	
	memset( g_cRecvCacheUart2,0,sizeof(g_cRecvCacheUart2) );
	memset( g_cRecvBufUart2,0,sizeof(g_cRecvBufUart2) );
	memset( g_cCmdBuf,0,sizeof(g_cCmdBuf) );
	
}









