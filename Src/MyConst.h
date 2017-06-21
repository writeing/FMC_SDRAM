/*******************************************************************
Company:   公司名
Engineer:  作者名
Create Date:  生成日期
File  Name:  文件名称
Description: 用于详细说明此程序文件完成的主要功能，与其他模块或函数的接口，输出值、取值范围、含义及参数间的控制、顺序、独立或依赖等关系
Function List:  // 主要函数列表，每条记录应包括函数名及功能简要说明

Revision: 修订信息
version: 版本号
Revision Date：修订日期
Modifier: 修订者姓名
Additional Comments: 修订的详细描述

********************************************************************/

#ifndef _MYCONST_H
#define _MYCONST_H
#include"math.h"
#define UART_PORT_DT0	   0x00//对外串口
#define UART_PORT_DT1	   0x01
#define UART_PORT_DT2	   0x02

#define UART_PORT_FPGA	   0x03
#define UART_PORT_ALL      0x04

#define FAILED          0x00
#define SUCCESSFUL      0x01


static double const TIC_PERIOD          = 0.05;     //10ms;
static double const TIC_FREQ            = 20.0;      //100Hz;



// F同步8秒以后，若F消失，仍认为有效
#define FRAME_VALID_TIME				10
#define NOBS            		80
#define MaxGpsSvID      		32
#define MinGpsSvID      		1
#define MinBD2SvID      		33
#define MaxBD2SvID      		46   
#define MAXSATELLITES       	46 
      
#define UART_MAX_CHANNELS   	12

#define UART_SEND_BUF_LENTH 	1000
#define UART_RECV_BUF_LENTH 	200
#define UART_RECV_FIELD_LENTH 	100
#define UART_RECV_CMD_BUF_CNT 	24

//#define BaseStation 1
//#define DirectMode 1

#define BUFFER_SIZE         ((unsigned int)0x0040000)

#define MAX_REACQU_CNT             6000  

#define MAX_CHANNELS                     	36     //重要修改 wy 2014
#define GPSBD2MAXCHANNELS                   24 
#define MAXSmoothEpoch         				12

#define MAXNSAT      36 				// 最大跟踪卫星数组 

//#define ROUND(x)        (int)floor((x)+0.5)
#define ROUND_U(x)  ((unsigned int)floor((x)+0.5))
#define P2_5        0.03125             /* 2^-5 */
#define P2_6        0.015625            /* 2^-6 */
#define P2_7        0.0078125  			/* 2^-6 */	
#define P2_11       4.882812500000000E-04 /* 2^-11 */
#define P2_15       3.051757812500000E-05 /* 2^-15 */
#define P2_17       7.629394531250000E-06 /* 2^-17 */
#define P2_19       1.907348632812500E-06 /* 2^-19 */
#define P2_20       9.536743164062500E-07 /* 2^-20 */
#define P2_21       4.768371582031250E-07 /* 2^-21 */
#define P2_23       1.192092895507810E-07 /* 2^-23 */
#define P2_24       5.960464477539063E-08 /* 2^-24 */
#define P2_27       7.450580596923828E-09 /* 2^-27 */
#define P2_29       1.862645149230957E-09 /* 2^-29 */
#define P2_30       9.313225746154785E-10 /* 2^-30 */
#define P2_31       4.656612873077393E-10 /* 2^-31 */
#define P2_32       2.328306436538696E-10 /* 2^-32 */
#define P2_33       1.164153218269348E-10 /* 2^-33 */
#define P2_35       2.910383045673370E-11 /* 2^-35 */
#define P2_38       3.637978807091710E-12 /* 2^-38 */
#define P2_39       1.818989403545856E-12 /* 2^-39 */
#define P2_40       9.094947017729280E-13 /* 2^-40 */
#define P2_43       1.136868377216160E-13 /* 2^-43 */
#define P2_48       3.552713678800501E-15 /* 2^-48 */
#define P2_50       8.881784197001252E-16 /* 2^-50 */
#define P2_55       2.775557561562891E-17 /* 2^-55 */
#define P2_66       1.355252715606881E-20 /* 2^-66 */
#define SC2RAD      3.1415926535898     /* semi-circle to radian (IS-GPS) */  

/*
#define FLASH_UPDATE_LENGTH						(4)

// Flash Address Parameters
#define FLASH_ADDRS 							(0x000000)								// Address of SPI Flash
#define FLASH_ADDR_EPH							(0x100000)								// Address of Flash to Program EPH
#define FLASH_ADDR_EPH_VALID					(0x170000)								// Address of Flash to Program EPH
#define FLASH_ADDR_POS							(0x180000)								// Address of Flash to Program POS
#define FLASH_ADDR_POS_VALID					(0x190000)								// Address of Flash to Program POS
#define FLASH_EPH								('E')									// Eph Flag
#define FLASH_POS								('P')									// Pos Flag

// Flash Operation Command
#define FLASH_CMD_LENGTH						(10)									// Command Length
#define FLASH_DATA_RET							(0x01)									// Flash Command Return Data
#define FLASH_NODATA_RET						(0x00)									// Flash Command Return No Data
#define FLASH_OPER_END							(0x01)									// Flash Operation Finished
#define FLASH_OPER_PRO							(0x00)									// Flash Operation Processed
#define FLASH_WRITE_ENABLE						(0x06)									// Write Enable
#define FLASH_READ_STATUS						(0x05)									// Read Status Register
#define FLASH_WRITE_STATUS						(0x01)									// Write Status Register
#define FLASH_READ_DATA							(0x03)									// Read Data
#define FLASH_PAGE_PROGRAM						(0x02)									// Page Program
#define FLASH_SECTOR_UNPROTECT					(0x39)									// Unprotect Sector
#define FLASH_SECTOR_ERASE						(0x20)									// Sector Erase
#define FLASH_BLOCK_ERASE						(0xD8)									// Block Erase
#define FLASH_PAGE_SIZE							(0x0100)								// Page size (byte)
#define FLASH_SECTOR_SIZE						(0x1000)								// Sector size (byte)
#define FLASH_BLOCK_SIZE						(0x10000)								// Block size (byte)
#define FLASH_PAGE_MASK							(0xFFFFFF00)							// Page mask
#define FLASH_DATA_VALID						(0xAA)									// Valid Data Flag
#define FLASH_DATA_INVALID						(0xFF)									// Invalid Data Flag

// Flash data frame header
#define FRAME_HEADER_EPH						(0xB0)									// Header of Eph
#define FRAME_HEADER_ALM						(0xB1)									// Header of Alm
#define FRAME_HEADER_IONO_BDMEOD1				(0xB2)									// Header of BD2 MEO D1 Iono
#define FRAME_HEADER_IONO_BDGEOD2				(0xB3)									// Header of BD2 GEO D2 Iono
#define FRAME_HEADER_IONO_GPS					(0xB4)									// Header of GPS Iono
#define FRAME_HEADER_UTC_BD2					(0xB5)									// Header of BD2 UTC
#define FRAME_HEADER_UTC_GPS					(0xB6)									// Header of GPS UTC
#define FRAME_HEADER_CLKMODEL_GPS				(0xB7)									// Header of GPS Clock Model
#define FRAME_HEADER_CLKMODEL_BD2				(0xB8)									// Header of BD2 Clock Model
#define FRAME_HEADER_NAVSTATE					(0xB9)									// Header of nav state
#define FRAME_HEADER_CONFIG						(0xBA)									// Header of config
#define FRAME_HEADER_EPH_TYPE					(0x07)									// num of headers in Eph struct
#define FRAME_HEADER_POS_TYPE					(0x04)									// num of headers in Pos struct
*/

static double const GEODESY_REFERENCE_ELLIPSE_WGS84_A    = 6.378137e6 ;
static double const GEODESY_REFERENCE_ELLIPSE_WGS84_B    =6356752.3142451793 ;
static double const GEODESY_REFERENCE_ELLIPSE_CGS2000_B    =6356752.3141403580 ;
static double const GEODESY_REFERENCE_ELLIPSE_CGS2000_E2 = 6.69438002290e-3;

static double const BD2GravConstant   = 3.986004418E14;  
//static double const BD2GravConstant   = 3.986005E14; 
static double const GPSGravConstant   = 3.986005E14;
static double const BIG_DOP           = 999.9; 
#ifndef MAXCHANNELS
#define MAXCHANNELS    12   
#endif
#define NCOMNOBS		100
#define SQR(x)      ((x)*(x))
#define VAR_POS     SQR(15.0) /* initial variance of receiver pos (m^2) */
#ifndef SPEED_OF_LIGHT
#define SPEED_OF_LIGHT        (2.99792458e8) 
#endif
#ifndef RECIP_SPEED_OF_LIGHT
#define RECIP_SPEED_OF_LIGHT        (3.3356409519815204e-9) 
#endif
#define RECIP_BD2_NW_WAVELENGTH 1.1806768000814751
#define BD2_NW_WAVELENGTH 0.84697183846671076
#define RECIP_BD2_NWToB1_WAVELENGTH 4.41040462427745686
#define B3_FREQUENCE  1268.52e6
#define B1_FREQUENCE  1561.098e6
#define L1_FREQUENCE  1575.42e6;    
#define RECIP_B1_WAVELENGTH  5.2072624188564474
#define RECIP_B3_WAVELENGTH  4.2313272604075984
#define RECIP_L1_WAVELENGTH  5.2550354685707275
#define RECIP_B3_FREQUENCE  7.8832024721722949E-010
#define RECIP_B1_FREQUENCE  6.4057477493405278E-010
#define RECIP_L1_FREQUENCE  6.3475136788919783E-010    
#define B1_WAVELENGTH  0.19203948631027648
#define L1_WAVELENGTH  0.19029367279836487
#define B3_WAVELENGTH  0.23633246460442089
#define IONOFREE_B1B3FREQ 2.9436817701459792

// Pi
//#ifndef PI
//#define PI        (3.1415926535897932384626433832795) 
//#endif
#ifndef RECIP_PI
#define RECIP_PI        (0.31830988618378997)
#endif
#ifndef TWO_PI
#define TWO_PI        (6.2831853071796) 
#endif
#define B3FREQSQUARE_B1FREQSQUARE 0.66028936614625655

// seconds in one week
static double const SECONDS_IN_WEEK   = 604800.00;
// seconds in half week
static double const SECONDS_IN_HALF_WEEK      = 302400.00;
static double const NEG_SECONDS_IN_HALF_WEEK  = -302400.00;
// seconds in one day
static double const SECONDS_IN_DAY    = 86400.00;
// seconds in one hour
static double const SECONDS_IN_HOUR   = 3600.00;
// seconds in one minute    
static double const SECONDS_IN_MINUTE = 60.00;
// day in four year

 
 static double const BD2WGS84oe        = 7.292115E-5;
static double const WGS84oe           = 7.2921151467E-5; 
static double const cos5 		= 0.996194698091745532295;
static double const sin5 		= -0.087155742747658173558;
static double const RECIP_P1023       = 0.00097751710654936461;
static double const RECIP_P2046       = 0.00048875855327468231;
static double const RECIP_P10230      = 9.7751710654936461e-005;
static double const RECIP_P65536      = 1.5258789062500000e-5;
#ifndef RAD2DEG
#define RAD2DEG   (57.295779513082320876798154814105)    //!< 180.0/PI
#endif
#ifndef DEG2RAD
#define DEG2RAD   (0.017453292519943295769236907684886)  //!< PI/180.0
#endif
#define PRMIC_NAV_BUF_COUNT         200

#define MAX_NAVBUFFERSIZE (100) 
#define WGS84OEMEA 2.4323877909897254e-013
#define BD2OEMEA 2.4323877420558726e-013
#endif

