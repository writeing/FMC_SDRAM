#include "LTE4G.h"
#include "FIFOuart.h"
#include "usart.h"

#define SOCKET_ID  0
#define OK 1 
#define ERROR -1

#define ACT_CMD        "AT+CGACT=1,1\r\n"
#define SHOWIP_CMD     "at+cgpaddr\r\n"
#define OPEN_CMD       "AT+QIOPEN=1,%d,\"TCP\",\"%s\",%d,9600,2\r\n"
#define EXIT_CMD       "+++++++"
#define CLOSE_CMD      "AT+QICLOSE=%d\r\n"



int port = 1234;

char ip[20] = "47,94,18,132";
uint8_t buff[1024];
int socket(int id)
{
	int count = 0;
	while(FIFO_UartReadBuffer(&gFIFO_Uart[0],buff,1024) == 0)
	{
		printf("wati LTE start\r\n");
		HAL_Delay(500);
	}
	printf("LTE started\r\n");
	act:  // ¼¤»îact»ñÈ¡IP
	if(count == 10)
	{
		printf("act error\r\n");
		return ERROR;
	}
	FIFO_UartSendBuffer(&gFIFO_Uart[0],(uint8_t *)ACT_CMD,strlen((char *)ACT_CMD));	
	HAL_Delay(1000);
	memset(buff,'\0',1024);
	while(FIFO_UartReadBuffer(&gFIFO_Uart[0],buff,1024) == 0)
	{
		printf("wati recv data\r\n");
		HAL_Delay(500);
	}
	if(strstr((char *)buff,"OK"))
	{
		// test
		FIFO_UartSendBuffer(&gFIFO_Uart[0],(uint8_t *)SHOWIP_CMD,sizeof(SHOWIP_CMD));		
		memset(buff,'\0',1024);
		HAL_Delay(1000);
		while(FIFO_UartReadBuffer(&gFIFO_Uart[0],buff,1024) == 0);
//		printf("wxc act = %s\t\n",buff);
		//test
		return SOCKET_ID;
	}
	else
	{
		count ++;
		goto act;
	}
}
int connect(int fd,const char *hostname,int port,int addr_len)
{
	char buf_cmd[100];	
	sprintf(buf_cmd,OPEN_CMD,fd,hostname,port);
//	printf("%s\r\n",buf_cmd);
	FIFO_UartSendBuffer(&gFIFO_Uart[0],(uint8_t *)buf_cmd,sizeof(buf_cmd));		
	memset(buff,'\0',1024);
	HAL_Delay(1000);
	while(FIFO_UartReadBuffer(&gFIFO_Uart[0],buff,1024) == 0);
	printf("recv = %s\r\n",buff);
//	while(FIFO_UartReadBuffer(&gFIFO_Uart[0],buff,1024) == 0);
//	printf("recv = %s\r\n",buff);
	if(strstr((char *)buff,"CONNECT"))
	{
		printf("CONNECT ok\r\n");
		return OK;
	}
	else
	{
		printf("CONNECT error\r\n");
		return ERROR;
	}
}
int close(int fd)
{
	FIFO_UartSendBuffer(&gFIFO_Uart[0],(unsigned char *)EXIT_CMD,sizeof(EXIT_CMD));
	char buf_cmd[100];
	HAL_Delay(1000);
	sprintf(buf_cmd,CLOSE_CMD,SOCKET_ID);
	FIFO_UartSendBuffer(&gFIFO_Uart[0],(unsigned char *)buf_cmd,sizeof(buf_cmd));	
	HAL_Delay(1000);
	while(FIFO_UartReadBuffer(&gFIFO_Uart[0],buff,1024) == 0);	
	if(strstr((char *)buff,"OK"))
	{
		memset(buff,'\0',1024);
		return OK;
	}
	else
	{
		return ERROR;
	}
	
}


int send(int fd,char* sendData,int length, int flag)
{	
	FIFO_UartSendBuffer(&gFIFO_Uart[0],(uint8_t *)sendData,length);
	return length;
}

int recv(int fd, char *buf, int32_t offset, int flag)
{	
	int len;
	flag = 30000;
	while(flag--)
	{		
		//printf("count = %d\r\n",(gFIFO_Uart[0].usRxCount));
		if((len = FIFO_UartReadBuffer(&gFIFO_Uart[0],(uint8_t *)buf,offset)) > 0 )
			 return len;
		HAL_Delay(1);
	}
	return -1;
}






