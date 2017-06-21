#ifndef __ICEGPS_UBLOX_H__
#define __ICEGPS_UBLOX_H__

typedef enum {
    GPS = 0,
    GPS_beidou,
    GNSS_TYPE_END
} UbloxGnssType;

typedef enum {
    RATE_1HZ = 0,
    RATE_2HZ,
    RATE_4HZ,
    RATE_5HZ,
    RATE_10HZ,
    RATE_END,
} UbloxRate;

typedef enum {
    BAUD9600 = 0,
    BAUD19200,
    BAUD38400,
    BAUD57600,
    BAUD115200,
		BAUD230400,
		BAUD460800,
		BAUD921600,
    BAUD_END
} UbloxBaudrate;

//extern void UbloxCloseGGA();
//extern void SetUbloxDataWithStation();
//extern void SetUbloxRate();
//extern void SetUbloxGnss();
//extern void SetUbloxUartBaudrate();
extern void SetUblox();


#endif 


