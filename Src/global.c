#include "global.h"
#include "struct.h"
#include "rtklib.h"
#include "TimeConvert.h"
#include "MyConst.h"

unsigned char		g_ucDirectMode = 0;
// RTK���
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


// �������

TDIFFOBSINFOSTRUCT			g_tDiffObsInfo 		__attribute__ ((section ("EXRAM")));								// �����Ϣ
TDIFFOBSINFOSTRUCT			g_tRevBaseInfo 		__attribute__ ((section ("EXRAM")));								// ���յ��Ļ�׼վ��Ϣ
unsigned short					g_usObsErrFlag;																												// �۲�������
unsigned short					g_usRtkRatio = 0;																											
unsigned short 					g_usRawBaseFlag = 0;																									// �����жϽ��ջ�׼վ�۲���������ϵı�־
unsigned short 					g_usRawRoverFlag = 0;																									// �����жϽ�������վ�۲���������ϵı�־
unsigned short 					g_usBaseRevFlag = 0;																									// �����жϽ��ջ�׼վ�۲������ݱ�����ϵı�־
unsigned short 					g_usRoverRevFlag = 0;																									// �����жϽ�������վ�۲������ݱ�����ϵı�־	

// �û���������
TBASEPOSSTRUCT					g_tBasePose;
TBASEPOSSTRUCT					g_tReadPosFromBase;
TUARTRMOSTRUCT					g_tUartRmo;
unsigned char						g_ucDataBuffer[1000];
unsigned short					g_usFlashProgrammeFlag = 0; //1���л�ģʽ��2������λ��
unsigned int						g_uiFlashProgrammerTic = 0;
unsigned short 					g_usUserState = 0;                                                   	//0:���У�1�����ڽ����ⲿ��������
unsigned char						g_ucStationType = 0;																									//0:Base,	1:Rover
unsigned short					g_usInitCompelet = 0;

//GNSSģ�����
unsigned short					g_usPpsInt = 0; //pps�жϱ�־
unsigned int						g_uiPpsTic = 0; //pps�ж�ʱ��TIC����
unsigned char						g_ucPpsSign = 0;

// ʱ�����
TBDUTCTIMEPARASTRUCT 		g_tBdUtcTimePara;               // BD UTC�ṹ��
TGPSUTCTIMEPARASTRUCT 	g_tGpsUtcTimePara;							// GPS UTC�ṹ��



// �������
TUARTRXCACHESTRUCT				g_tUartRover[UART_RECV_CMD_BUF_CNT]											__attribute__ ((section ("EXRAM"))) ;
unsigned short						g_usRoverRxCurPos = 0;	
unsigned short						g_usRoverRxCmdPos = 0;
TUARTRXCACHESTRUCT				g_tUartBase[UART_RECV_CMD_BUF_CNT] 											__attribute__ ((section ("EXRAM"))) ;
unsigned short						g_usBaseRxCurPos = 0;
unsigned short						g_usBaseRxCmdPos = 0;


char 											g_cSendBufUart0[UART_SEND_BUF_LENTH] 										__attribute__ ((section ("EXRAM"))) = {0};          						// ����0���ͻ��� 		
volatile unsigned short 	g_usSendSetPosUart0 = 0;	
volatile unsigned short 	g_usSendCurPosUart0 = 0; 
char 											g_cFieldBuf[UART_RECV_FIELD_LENTH];// 											__attribute__ ((section ("EXRAM")));
char 											g_cSendBuf[UART_SEND_BUF_LENTH];// 												__attribute__ ((section ("EXRAM")));
unsigned char 						g_ucSendBuf[UART_SEND_BUF_LENTH]																				__attribute__ ((section ("EXRAM")));
unsigned short 						g_usSendIndex = 0; 

double 										dTxBuffer[BUFFER_SIZE] 																		__attribute__ ((section ("EXRAM")));
double 										dRxBuffer[BUFFER_SIZE] 																		__attribute__ ((section ("EXRAM")));


char 											g_cSendBufUart1[UART_SEND_BUF_LENTH] 										__attribute__ ((section ("EXRAM"))) = {0};          						// ����1���ͻ���
volatile unsigned short 	g_usSendSetPosUart1 = 0;
volatile unsigned short 	g_usSendCurPosUart1 = 0; 


char 											g_cSendBufUart2[UART_SEND_BUF_LENTH] 										__attribute__ ((section ("EXRAM")))= {0};          						// ����1���ͻ���
volatile unsigned short 	g_usSendSetPosUart2 = 0;
volatile unsigned short 	g_usSendCurPosUart2 = 0;  

char 						g_cRecvCacheUart2[UART_RECV_CMD_BUF_CNT][UART_RECV_BUF_LENTH] 		__attribute__ ((section ("EXRAM")))= {0};      // ���ڽ���CACHE
char 						g_cRecvBufUart2[UART_RECV_BUF_LENTH];// 															__attribute__ ((section ("EXRAM")))= {0};																// ���ڽ���BUFF
char 						g_cCmdBuf[UART_RECV_BUF_LENTH];//																		__attribute__ ((section ("EXRAM")));       																		// ����ָ����ջ���
unsigned char   g_ucCmdBuf[UART_RECV_BUF_LENTH];
unsigned short						g_usRecvFlagUart2 = 0;
volatile unsigned short		g_usRecvCntUart2 = 0; 																					// �������ݼ���
volatile unsigned short 	g_usRecvCmdSetPosUart2 = 0;      																// ���ڱ������ݻ���λ��      
volatile unsigned short 	g_usRecvCmdCurPosUart2 = 0;																			//���ڶ�ȡ���ݻ���λ��



void InitGlobal( void )
{
	
	// �������
	memset( &g_tDiffObsInfo,0,sizeof(g_tDiffObsInfo) );
	memset( &g_tRevBaseInfo,0,sizeof(g_tRevBaseInfo) );
	
	// �û���������
	memset( &g_tBasePose,0,sizeof(g_tBasePose) );
	memset( &g_tReadPosFromBase,0,sizeof(g_tReadPosFromBase) );
	

		
	// �������		
	
	
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









