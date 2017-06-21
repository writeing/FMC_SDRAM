#ifndef __SPB_H
#define __SPB_H

extern void MegGpsTime( UART_HandleTypeDef *huart );
extern void MegDops( UART_HandleTypeDef *huart );
extern void MegPosECEF( UART_HandleTypeDef *huart );
extern void MegPosLLH( UART_HandleTypeDef *huart );
extern void MegVelECEF( UART_HandleTypeDef *huart );
extern void MegVelENU( UART_HandleTypeDef *huart );
extern unsigned short MegHEARTBEAT( UART_HandleTypeDef *huart );
	
#endif