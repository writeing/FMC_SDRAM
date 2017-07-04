#ifndef _LTE_4G_H_
#define _LTE_4G_H_
#include "string.h"
#include "stdint.h"
#include "stm32f7xx_hal.h"
extern int port;

extern char ip[20];

extern int socket(int id);

extern int connect(int fd,const char *hostname,int port,int addr_len);

extern int close(int fd);
	
extern int send(int fd,char* sendData,int length, int flag);

extern int recv(int fd, char *buf, int32_t offset, int flag);
#endif





