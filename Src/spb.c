#include "main.h"
#include "usart.h"
#include "global.h"
#include "rtklib.h"
#include "MyConst.h"
#include "spb.h"

//unsigned short	usCrcCheckSum = 0;

static const unsigned short crc16tab[256]  = {
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
  0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
  0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
  0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
  0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
  0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
  0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
  0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
  0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
  0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
  0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
  0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
  0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
  0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
  0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
  0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
  0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
  0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
  0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
  0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
  0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
  0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
  0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

/** Calculate CCITT 16-bit Cyclical Redundancy Check (CRC16).
 *
 * This implementation uses parameters used by XMODEM i.e. polynomial is:
 * \f[
 *   x^{16} + x^{12} + x^5 + 1
 * \f]
 * Mask 0x11021, not reversed, not XOR'd
 * (there are several slight variants on the CCITT CRC-16).
 *
 * \param buf Array of data to calculate CRC for
 * \param len Length of data array
 * \param crc Initial CRC value
 *
 * \return CRC16 value
 */
unsigned short crc16_ccitt(const unsigned char *buf, unsigned int len, unsigned short crc)
{
  for (unsigned int i = 0; i < len; i++)
    crc = (crc << 8) ^ crc16tab[((crc >> 8) ^ *buf++) & 0x00FF];
  return crc;
}

void MegGpsTime( UART_HandleTypeDef *huart )
{	
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n,ucFixStatus;
	int 						iGpsWeek,iNsec;
	unsigned int		uiMsec;
	double 					dRoverSecond;
	unsigned short	usCrcCheckSum = 0;
	
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	g_usSendIndex = 0;
	
	// 未定位值直接返回
	if(ptRtk->sol.stat==0)
		return;
	
	// header
	*q++=0x55;
	
	// message type
	*q++=0x00;
	*q++=0x01;
	
	// sender
	*q++=0x22;
	*q++=0x22;
	
	// length
	q+=1;
	
	// GPS时 周
	setU2(q,(unsigned short)iGpsWeek); 					q+=2;
	
	// GPS时 ms
	uiMsec 	= round(dRoverSecond*1000);
	iNsec 	= (int)(dRoverSecond*1000.0 - uiMsec)*1e6;
	setU4(q,uiMsec); 														q+=4;
	setI4(q,iNsec); 														q+=4;
	
	
	if ( (ptRtk->sol.stat >= 0x01)&&(ptRtk->sol.ns <= 0x04) )
	{
		ucFixStatus = 0x00;
	}
	else if (ptRtk->sol.stat==5)
	{
		ucFixStatus = 0x00;
	}
	else if(ptRtk->sol.stat==2)
	{
		ucFixStatus = 0x02;
	}
	else if(ptRtk->sol.stat==1)
	{
		ucFixStatus = 0x01;
	}
	
	setU1(q,ucFixStatus); 											q+=1;

	// length	
	n=(int)(q-g_cSendBuf)+2;    
	setU1(g_cSendBuf+5,(unsigned short)(n-8));
	
	// crc check
	usCrcCheckSum = crc16_ccitt(g_cSendBuf+1,n-3,usCrcCheckSum);
	setU2(g_cSendBuf+n-2,usCrcCheckSum);

	g_usSendIndex = n;
	UartSend(huart, g_cSendBuf, n);
}

void MegDops( UART_HandleTypeDef *huart )
{	
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n;
	int 						iGpsWeek;
	unsigned int		uiMsec;
	double 					dRoverSecond;
	unsigned short	usCrcCheckSum = 0;
	
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	g_usSendIndex = 0;
	
	// 未定位值直接返回
	if(ptRtk->sol.stat==0)
		return;
	
	// header
	*q++=0x55;
	
	// message type
	*q++=0x06;
	*q++=0x02;
	
	// sender
	*q++=0x22;
	*q++=0x22;
	
	// length
	q+=1;
		
	// GPS时 ms
	uiMsec 	= round(dRoverSecond*1000);
	setU4(q,uiMsec); 														q+=4;
	
	setU2(q,(unsigned short)(g_dDop[0]*100+0.5)); 			q+=2;     //gdop
	setU2(q,(unsigned short)(g_dDop[1]*100+0.5));				q+=2;     //pdop
	setU2(q,(unsigned short)(g_dDop[4]*100+0.5));				q+=2;     //tdop
	setU2(q,(unsigned short)(g_dDop[2]*100+0.5)); 			q+=2;     //hdop
	setU2(q,(unsigned short)(g_dDop[3]*100+0.5));				q+=2;     //vdop
	
	// length	
	n=(int)(q-g_cSendBuf)+2;    
	setU1(g_cSendBuf+5,(unsigned short)(n-8));
	
	// crc check
	usCrcCheckSum = crc16_ccitt(g_cSendBuf+1,n-3,usCrcCheckSum);
	setU2(g_cSendBuf+n-2,usCrcCheckSum);

	g_usSendIndex = n;
	UartSend(huart, g_cSendBuf, n);
}

void MegPosECEF( UART_HandleTypeDef *huart )
{
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n,ucFixStatus;
	int 						iGpsWeek;
	unsigned int		uiMsec;
	double 					dRoverSecond;
	unsigned short	usCrcCheckSum = 0;
	
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	g_usSendIndex = 0;
	
	// 未定位值直接返回
	if(ptRtk->sol.stat==0)
		return;
	
	if ( (ptRtk->sol.stat >= 0x01)&&(ptRtk->sol.ns <= 0x04) )
	{
		ucFixStatus = 0x00;
	}
	else if (ptRtk->sol.stat==5)
	{
		ucFixStatus = 0x00;
	}
	else if(ptRtk->sol.stat==2)
	{
		ucFixStatus = 0x02;
	}
	else if(ptRtk->sol.stat==1)
	{
		ucFixStatus = 0x01;
	}
	
	// header
	*q++=0x55;
	
	// message type
	*q++=0x00;
	*q++=0x02;
	
	// sender
	*q++=0x22;
	*q++=0x22;
	
	// length
	q+=1;
	
	// GPS时 ms
	uiMsec 	= round(dRoverSecond*1000);
	setU4(q,uiMsec); 														q+=4;
	
	// ECEF坐标
	setR8(q,ptRtk->sol.rr[0]); 									q+=8;
	setR8(q,ptRtk->sol.rr[1]); 									q+=8;
	setR8(q,ptRtk->sol.rr[2]); 									q+=8;
	
	// 定位精度
	setU2(q,0); 																q+=2;
	
	// 参与定位卫星数
	setU1(q,ptRtk->sol.ns); 										q+=1;
	
	// 定位卫星数
	setU1(q,ucFixStatus); 											q+=1;
	
	// length	
	n=(int)(q-g_cSendBuf)+2;    
	setU1(g_cSendBuf+5,(unsigned short)(n-8));
	
	// crc check
	usCrcCheckSum = crc16_ccitt(g_cSendBuf+1,n-3,usCrcCheckSum);
	setU2(g_cSendBuf+n-2,usCrcCheckSum);

	g_usSendIndex = n;
	UartSend(huart, g_cSendBuf, n);

}

void MegPosLLH( UART_HandleTypeDef *huart )
{
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n,ucFixStatus;
	int 						iGpsWeek;
	unsigned int		uiMsec;
	double 					dRoverSecond,dPos[3];
	unsigned short	usCrcCheckSum = 0;
	
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	ecef2pos(ptRtk->sol.rr,dPos);
	g_usSendIndex = 0;
	
	// 未定位值直接返回
	if(ptRtk->sol.stat==0)
		return;
	
	if ( (ptRtk->sol.stat >= 0x01)&&(ptRtk->sol.ns <= 0x04) )
	{
		ucFixStatus = 0x00;
	}
	else if (ptRtk->sol.stat==5)
	{
		ucFixStatus = 0x00;
	}
	else if(ptRtk->sol.stat==2)
	{
		ucFixStatus = 0x02;
	}
	else if(ptRtk->sol.stat==1)
	{
		ucFixStatus = 0x01;
	}
	
	// header
	*q++=0x55;
	
	// message type
	*q++=0x01;
	*q++=0x02;
	
	// sender
	*q++=0x22;
	*q++=0x22;
	
	// length
	q+=1;
	
	
	// GPS时 ms
	uiMsec 	= round(dRoverSecond*1000);
	setU4(q,uiMsec); 														q+=4;
	
	// ECEF坐标
	setR8(q,dPos[0]*R2D); 											q+=8;
	setR8(q,dPos[1]*R2D); 											q+=8;
	setR8(q,dPos[2]); 													q+=8;
	
	// 定位精度
	setU2(q,0); 																q+=2;
	setU2(q,0); 																q+=2;
	
	// 参与定位卫星数
	setU1(q,ptRtk->sol.ns); 										q+=1;
	
	// 定位卫星数
	setU1(q,ucFixStatus); 											q+=1;
	
	// length	
	n=(int)(q-g_cSendBuf)+2;    
	setU1(g_cSendBuf+5,(unsigned short)(n-8));
	
	// crc check
	usCrcCheckSum = crc16_ccitt(g_cSendBuf+1,n-3,usCrcCheckSum);
	setU2(g_cSendBuf+n-2,usCrcCheckSum);

	g_usSendIndex = n;
	UartSend(huart, g_cSendBuf, n);

}

void MegVelECEF( UART_HandleTypeDef *huart )
{
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n,ucFixStatus;
	int 						iGpsWeek;
	unsigned int		uiMsec;
	double 					dRoverSecond;
	unsigned short	usCrcCheckSum = 0;
	
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	g_usSendIndex = 0;
	
	// 未定位值直接返回
	if(ptRtk->sol.stat==0)
		return;
	
	if ( (ptRtk->sol.stat >= 0x01)&&(ptRtk->sol.ns <= 0x04) )
	{
		ucFixStatus = 0x00;
	}
	else if (ptRtk->sol.stat==5)
	{
		ucFixStatus = 0x00;
	}
	else if(ptRtk->sol.stat==2)
	{
		ucFixStatus = 0x02;
	}
	else if(ptRtk->sol.stat==1)
	{
		ucFixStatus = 0x01;
	}
	
	// header
	*q++=0x55;
	
	// message type
	*q++=0x04;
	*q++=0x02;
	
	// sender
	*q++=0x22;
	*q++=0x22;
	
	// length
	q+=1;
	
	
	// GPS时 ms
	uiMsec 	= round(dRoverSecond*1000);
	setU4(q,uiMsec); 														q+=4;
	
	// ECEF速度
	setI4(q,round(ptRtk->sol.rr[3]*1000)); 			q+=4;
	setI4(q,round(ptRtk->sol.rr[4]*1000)); 			q+=4;
	setI4(q,round(ptRtk->sol.rr[5]*1000)); 			q+=4;
	
	// 定速精度
	setU2(q,0); 																q+=2;
	
	// 参与定位卫星数
	setU1(q,ptRtk->sol.ns); 										q+=1;
	
	// 定位卫星数
	setU1(q,ucFixStatus); 											q+=1;
	
	// length	
	n=(int)(q-g_cSendBuf)+2;    
	setU1(g_cSendBuf+5,(unsigned short)(n-8));
	
	// crc check
	usCrcCheckSum = crc16_ccitt(g_cSendBuf+1,n-3,usCrcCheckSum);
	setU2(g_cSendBuf+n-2,usCrcCheckSum);

	g_usSendIndex = n;
	UartSend(huart, g_cSendBuf, n);

}

void MegVelENU( UART_HandleTypeDef *huart )
{
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n,ucFixStatus;
	int 						iGpsWeek;
	unsigned int		uiMsec;
	double 					dRoverSecond,dPos[3],dVelEnu[3];
	unsigned short	usCrcCheckSum = 0;
	
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	g_usSendIndex = 0;
	
	// 计算ENU速度
	ecef2pos(ptRtk->sol.rr,dPos);
	ecef2enu(dPos,ptRtk->sol.rr+3,dVelEnu);
	
	// 未定位值直接返回
	if(ptRtk->sol.stat==0)
		return;
	
	if ( (ptRtk->sol.stat >= 0x01)&&(ptRtk->sol.ns <= 0x04) )
	{
		ucFixStatus = 0x00;
	}
	else if (ptRtk->sol.stat==5)
	{
		ucFixStatus = 0x00;
	}
	else if(ptRtk->sol.stat==2)
	{
		ucFixStatus = 0x02;
	}
	else if(ptRtk->sol.stat==1)
	{
		ucFixStatus = 0x01;
	}
	
	// header
	*q++=0x55;
	
	// message type
	*q++=0x05;
	*q++=0x02;
	
	// sender
	*q++=0x22;
	*q++=0x22;
	
	// length
	q+=1;
	
	
	// GPS时 ms
	uiMsec 	= round(dRoverSecond*1000);
	setU4(q,uiMsec); 														q+=4;
	
	// NED速度
	setI4(q,round(dVelEnu[1]*1000)); 						q+=4;
	setI4(q,round(dVelEnu[0]*1000)); 						q+=4;
	setI4(q,round(0-dVelEnu[2]*1000)); 					q+=4;
	
	
	// 定速精度
	setU2(q,0); 																q+=2;
	setU2(q,0); 																q+=2;
	
	// 参与定位卫星数
	setU1(q,ptRtk->sol.ns); 										q+=1;
	
	// 定位卫星数
	setU1(q,ucFixStatus); 											q+=1;
	
	// length	
	n=(int)(q-g_cSendBuf)+2;    
	setU1(g_cSendBuf+5,(unsigned short)(n-8));
	
	// crc check
	usCrcCheckSum = crc16_ccitt(g_cSendBuf+1,n-3,usCrcCheckSum);
	setU2(g_cSendBuf+n-2,usCrcCheckSum);

	g_usSendIndex = n;
	UartSend(huart, g_cSendBuf, n);

}

unsigned short MegHEARTBEAT( UART_HandleTypeDef *huart )
{
	rtk_t 					*ptRtk = &g_tRtk;
	unsigned char 	*q = g_cSendBuf,n;
	double 					dRoverSecond;
	unsigned short	usCrcCheckSum = 0;
	int							iGpsWeek;
	
	dRoverSecond = time2gpst(ptRtk->sol.time,&iGpsWeek);
	
	//if( fmod(dRoverSecond,1.0)>0.2 && fmod(dRoverSecond,1.0)<0.90)
	//	return 0;
	
	// 未定位值直接返回
	//if(ptRtk->sol.stat==0)
	//	return 0;
	
	// header
	*q++=0x55;
	
	// message type
	*q++=0xFF;
	*q++=0xFF;
	
	// sender
	*q++=0x22;
	*q++=0x22;
	
	// length
	q+=1;
	
	setU4(q,0x0); 						q+=4;
	
	
	// length	
	n=(int)(q-g_cSendBuf)+2;    
	setU1(g_cSendBuf+5,(unsigned char)(n-8));
	
	// crc check
	usCrcCheckSum = crc16_ccitt(g_cSendBuf+1,n-3,0);
	
	setU2(g_cSendBuf+n-2,usCrcCheckSum);
	g_usSendIndex = n;

	UartSend(huart, g_cSendBuf, n);
	return 1;
	

}