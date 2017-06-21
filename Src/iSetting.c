#include "main.h"
#include "usart.h"
#include "global.h"
#include "rtklib.h"
#include "TimeConvert.h"
#include "MyConst.h"
#define BAUD_END 100
//////////////////////////////////////////////////////////////////////////
////////////////////////////////??????//////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void Field(char *pcInputSentence,char *pcOutputBuf,unsigned short field_number)
{
    unsigned short index                = 1; 
    unsigned short index1               = 0;
    unsigned short current_field_number = 0;
    unsigned short string_length        = strlen(pcInputSentence);
    char return_string[UART_RECV_FIELD_LENTH] = {0};

	if(string_length>=UART_RECV_BUF_LENTH)
	{
		string_length = 0;
	}

    memset (pcOutputBuf, 0, UART_RECV_FIELD_LENTH);
	
    while( current_field_number < field_number && index < string_length )
    {
        if ( pcInputSentence[ index ] == ',' || pcInputSentence[ index ] == '*' )
        {
            current_field_number++;
        }
        index++;
    }
    
    if ( current_field_number == field_number )
    {
        while( index < string_length      &&
            pcInputSentence[ index ] != ',' &&
            pcInputSentence[ index ] != '*' &&
            pcInputSentence[ index ] != 0x00 )
        {
            if(index1 >= UART_RECV_FIELD_LENTH)
            {
				//puts("Get UART Field Failed!!");
				return;
            }
            return_string[index1]= pcInputSentence[ index ];
            index++;
            index1++;
        }
    }
	// LogStr(return_string);
    memcpy(pcOutputBuf,return_string,index1);
}
extern void setcs1(unsigned char *buff, int len);
extern kal_char* build_date_time(void);
void SendSetting2PC()
{
	unsigned char sendBuffer[1024]={0};
	unsigned char *q=sendBuffer;
	//unsigned char *buildDateTime=build_date_time();
	int len = 0;
	// header
	*q++=0xB5;
	*q++=0x62;
	// class
	*q++=0x01;
	// ID
	*q++=0x19;
	// length
	q+=2;	

//	setU1(q,g_tBasePose.uiStationType); 		q+=1;
//	setU1(q,g_tBasePose.uiOutputType); 			q+=1;
//	setU1(q,g_tBasePose.uiRtkRate); 			q+=1;
//	setU1(q,g_tBasePose.uiGNSSType); 			q+=1;
//	setU1(q,g_tBasePose.uiRtkMode); 			q+=1;
//	setU1(q,g_tBasePose.uiRtkOutputBaudrate); 	q+=1;
//	setU1(q,gRadioStruct.enableFlag); 			q+=1;
//	setU2(q,gRadioStruct.netId); 				q+=2;
//	setU2(q,gRadioStruct.channels); 			q+=2;
//	setU1(q,gRadioStruct.radioBaudrate); 		q+=1;
//	setU1(q,gNmeaOutChoose); 			q+=1;
//	setU1(q,gUbxOutChoose); 			q+=1;
//	setU1(q,gSbpOutChoose); 			q+=1;
//	setU1(q,gSaveOriginalDataFlag); 			q+=1;
//	setU1(q,gSaveResultDataFlag); 			q+=1;

	//memcpy(q,buildDateTime,strlen(buildDateTime));	q+=strlen(buildDateTime);//??????

	// LogStr(buildDateTime);
	// LogStr("strlen(buildDateTime) = %d",strlen(buildDateTime));

	len=(int)(q-sendBuffer)+2;    
	setU2(sendBuffer+4,(unsigned short)(len-8));
	//setcs1(sendBuffer,len);

	UartSendBuffer(&Uart3Handle, sendBuffer, len);
}

U8 gSetFlag = 0;
void ReceiveIntoPCSet(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int setFlag = 0;
				
	Field(RecvBuffer, cFieldBuf, field_number);
	setFlag = atoi( cFieldBuf );
	if(setFlag == 0 || setFlag == 1)
	{
		// LogStr(cFieldBuf);
		gSetFlag = setFlag;
	}
	if (gSetFlag == 1)
	{
		SendSetting2PC();					
	}
}

void ReceiveBasePosSetting(char *RecvBuffer)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};

	Field( RecvBuffer,cFieldBuf,2 );
	g_tBasePose.dUserSetX = atof( cFieldBuf );
	// LogStr(cFieldBuf);
	Field( RecvBuffer,cFieldBuf,3 );
	g_tBasePose.dUserSetY = atof( cFieldBuf );
	// LogStr(cFieldBuf);
	Field( RecvBuffer,cFieldBuf,4);
	g_tBasePose.dUserSetZ = atof( cFieldBuf );
	// LogStr(cFieldBuf);
	Field( RecvBuffer,cFieldBuf,5 );
	g_tBasePose.uiUserSetFlag = atoi( cFieldBuf );
	// LogStr(cFieldBuf);
	//WriteBaseStationUserSetPos();
}

void ReceiveStationSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int stationType = 0;
	unsigned int oldStationType = g_tBasePose.uiStationType;

	Field(RecvBuffer, cFieldBuf, field_number);
	stationType = atoi( cFieldBuf );
	if(stationType == 0 || stationType == 1)
	{
		// LogStr(cFieldBuf);
		g_tBasePose.uiStationType = stationType;
		//WriteStationType();
		g_tBasePose.uiStationType = oldStationType;
	}
}

void ReceiveOutputSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int outputType = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	outputType = atoi( cFieldBuf );
	//if(outputType >= 0 && outputType < OUTPUT_TYPE_END)
	{
		// LogStr(cFieldBuf);
		g_tBasePose.uiOutputType = outputType;
	//	WriteOutputType();			
	}
}

void ReceiveOutputRateSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int outputRate = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	outputRate = atoi( cFieldBuf );
//	if(outputRate >= 0 && outputRate < RATE_END)
	{
		// LogStr(cFieldBuf);
	//	g_tBasePose.uiRtkRate = outputRate;
//		WriteRtkRate();			
	}
}

void ReceiveGnssSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int gnssType = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	gnssType = atoi( cFieldBuf );
//	if(gnssType >= 0 && gnssType < GNSS_TYPE_END)
//	{
//		// LogStr(cFieldBuf);
//		g_tBasePose.uiGNSSType = gnssType;
//		WriteGNSSType();
//	}
	
}

void ReceiveRtkModeSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int rtkMode = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	rtkMode = atoi( cFieldBuf );
	if(rtkMode >= 0 && rtkMode < 3)
	{
//		// LogStr(cFieldBuf);
//		g_tBasePose.uiRtkMode = rtkMode;
//		WriteRtkMode();
//		SetRtkMode();
	}	
}

void ReceiveOutBaudrateSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int outBaudrate = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	outBaudrate = atoi( cFieldBuf );
	if(outBaudrate >= 0 && outBaudrate < BAUD_END)
	{
		// LogStr(cFieldBuf);
		//g_tBasePose.uiRtkOutputBaudrate = outBaudrate;
		//WriteRtkOutputBaudrate();			
	}
}

void ReceiveRadioEnbleSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int radioEnble = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	radioEnble = atoi( cFieldBuf );
	if(radioEnble == 0 || radioEnble == 1)
	{
		// LogStr(cFieldBuf);
		//gRadioStruct.enableFlag = radioEnble;
	//	WriteRadioEnableSetting();		
	}	
}

void ReceiveRadioNetIDSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int netId = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	netId = atoi( cFieldBuf );
	if(netId >= 0 && netId <= 499)
	{
		// LogStr(cFieldBuf);
		//gRadioStruct.netId = netId;
		//WriteRadioNetIdSetting();	
	}
}

void ReceiveRadioChannelsSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int channels = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	channels = atoi( cFieldBuf );
	if(channels >= 5 || channels <= 30)
	{
		// LogStr(cFieldBuf);
		//gRadioStruct.channels = channels;
		//WriteRadioChannelsSetting();			
	}
}

void ReceiveExtRadioSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int radioBaudrate = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	radioBaudrate = atoi( cFieldBuf );
	if(radioBaudrate >= 0 && radioBaudrate < BAUD_END)
	{
		// LogStr(cFieldBuf);
		//gRadioStruct.radioBaudrate = radioBaudrate;
		//WriteExtRadioBaudrate();			
	}
}
unsigned int gNmeaOutChoose = 0;
void ReceiveNmeaOutChooseSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int nmeaOutChoose = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	nmeaOutChoose = atoi( cFieldBuf );
	if (nmeaOutChoose >= 0)
	{
		gNmeaOutChoose = nmeaOutChoose;	
		//WriteNmeaOutChoose();	
	}
}
unsigned int gSbpOutChoose = 0;
void ReceiveSbpOutChooseSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int sbpOutChoose = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	sbpOutChoose = atoi( cFieldBuf );
	if (sbpOutChoose >= 0)
	{
		gSbpOutChoose = sbpOutChoose;
		//WriteSbpOutChoose();				
	}
}
unsigned int gUbxOutChoose=0;
void ReceiveUbxOutChooseSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int ubxOutChoose = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	ubxOutChoose = atoi( cFieldBuf );
	if (ubxOutChoose >= 0)
	{
		gUbxOutChoose = ubxOutChoose;
		//WriteUbxOutChoose();					
	}
}
unsigned int gSaveOriginalDataFlag = 0;
void ReceiveSaveOriginalDataSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int saveOriginalDataFlag = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	saveOriginalDataFlag = atoi( cFieldBuf );
	if (saveOriginalDataFlag == 0 || saveOriginalDataFlag == 1)
	{
		gSaveOriginalDataFlag = saveOriginalDataFlag;
		//WriteSaveOriginalDataSet();					
	}
}
unsigned int gSaveResultDataFlag;

void ReceiveSaveResultDataSetting(char *RecvBuffer, unsigned short field_number)
{
	char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};
	unsigned int saveResultDataFlag = 0;

	Field(RecvBuffer, cFieldBuf, field_number);
	saveResultDataFlag = atoi( cFieldBuf );
	if (saveResultDataFlag == 0 || saveResultDataFlag == 1)
	{
		gSaveResultDataFlag = saveResultDataFlag;
		//WriteSaveResultSet();					
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////??RTk??////////////////////////////
////////////////////////////////////////////////////////////////////////// 
void ReadRTKSetCmd( char *RecvBuffer, unsigned short usLenth)
{   			
    char	cFieldBuf[UART_RECV_FIELD_LENTH]={0};

	// if ( (RecvBuffer[usLenth - 1] == 0x0d) && (RecvBuffer[usLenth] == 0x0a))
	{
		// LogStr(RecvBuffer);
        if(strncmp(RecvBuffer,"$ICERTK",7) == 0)
        {
			Field( RecvBuffer,cFieldBuf,1 );
			// LogStr(cFieldBuf);
			if(strcmp(cFieldBuf,"SET") == 0)
			{
				ReceiveIntoPCSet(RecvBuffer,2);
			}
			else if(strcmp(cFieldBuf,"POS") == 0)
			{
				ReceiveBasePosSetting(RecvBuffer);
			}
			else if(strcmp(cFieldBuf,"STATION") == 0)
			{
				ReceiveStationSetting(RecvBuffer,2);
			}
			else if(strcmp(cFieldBuf,"OUT") == 0)
			{
				ReceiveOutputSetting(RecvBuffer,2);					
			}
			else if(strcmp(cFieldBuf,"RATE") == 0)
			{
				ReceiveOutputRateSetting(RecvBuffer,2);	
			}
			else if(strcmp(cFieldBuf,"GNSS") == 0)
			{
				ReceiveGnssSetting(RecvBuffer,2);	
			}
			else if(strcmp(cFieldBuf,"MODE") == 0)
			{
				ReceiveRtkModeSetting(RecvBuffer,2);	
			}	
			else if(strcmp(cFieldBuf,"BAUD") == 0)
			{
				ReceiveOutBaudrateSetting(RecvBuffer,2);			
			}	
			else if(strcmp(cFieldBuf,"RADIO") == 0)
			{
				ReceiveRadioEnbleSetting(RecvBuffer,2);
				ReceiveRadioNetIDSetting(RecvBuffer,4);
				ReceiveRadioChannelsSetting(RecvBuffer,6);			
			}
			else if(strcmp(cFieldBuf,"EXTRADIO") == 0)
			{
				ReceiveExtRadioSetting(RecvBuffer,2);	
			}
			else if(strcmp(cFieldBuf,"NMEA") == 0)
			{
				ReceiveNmeaOutChooseSetting(RecvBuffer,2);			
			}		
			else if(strcmp(cFieldBuf,"SBP") == 0)
			{	
				ReceiveSbpOutChooseSetting(RecvBuffer,2);				
			}	
			else if(strcmp(cFieldBuf,"UBX") == 0)
			{
				ReceiveUbxOutChooseSetting(RecvBuffer,2);
			}
			else if(strcmp(cFieldBuf,"ORIDATA") == 0)
			{
				ReceiveSaveOriginalDataSetting(RecvBuffer,2);
			}
			else if(strcmp(cFieldBuf,"RESULTS") == 0)
			{
				ReceiveSaveResultDataSetting(RecvBuffer,2);
			}
			else if(strcmp(cFieldBuf,"SETTINGS") == 0)
			{
				ReceiveStationSetting(RecvBuffer,2);
				ReceiveOutputSetting(RecvBuffer,3);
				ReceiveOutputRateSetting(RecvBuffer,4);
				ReceiveGnssSetting(RecvBuffer,5);
				ReceiveRtkModeSetting(RecvBuffer,6);
				ReceiveOutBaudrateSetting(RecvBuffer,7);
				ReceiveRadioEnbleSetting(RecvBuffer,8);
				ReceiveRadioNetIDSetting(RecvBuffer,9);
				ReceiveRadioChannelsSetting(RecvBuffer,10);
				ReceiveExtRadioSetting(RecvBuffer,11);
				ReceiveNmeaOutChooseSetting(RecvBuffer,12);
				ReceiveSbpOutChooseSetting(RecvBuffer,13);
				ReceiveUbxOutChooseSetting(RecvBuffer,14);
				ReceiveSaveOriginalDataSetting(RecvBuffer,15);
				ReceiveSaveResultDataSetting(RecvBuffer,16);	
			}	
        }		
	}
}
