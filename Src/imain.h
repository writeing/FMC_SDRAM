
#ifndef __IMAIN__
#define __IMAIN__

extern unsigned char RtkUart_1_ReadHandler(void);
extern unsigned char RtkUart_UBLOX_ReadHandler(void);
extern unsigned char RtkUart_USB_ReadHandler(void);

extern void icegpsMain(void);
extern void processRtk(void);

#endif
