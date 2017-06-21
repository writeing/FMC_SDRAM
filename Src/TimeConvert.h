

#ifndef _TIMECONVERT_H
#define _TIMECONVERT_H

void UtcParaInit();
/*******************************************************
*         定义结构体                                 *
*******************************************************/

// 北斗2UTC时结构体
typedef struct _tBdUtcTimeParaStruct
{
    unsigned short vflg;                   
    short  dtls;               
    unsigned short dn;                
    short  dtlsf;         
    unsigned short wnlsf;    
    double A0;
    double A1;     
}TBDUTCTIMEPARASTRUCT;

// GPS UTC时结构体
typedef struct _tGpsUtcTimeParaStruct
{
    unsigned short vflg;                            // 有效标志   
    short dtls;                             // Cumulative past leap seconds
    unsigned short dn;                              // Day of week (1-7) when dtlsf becomes effective
    short dtlsf;                            // Scheduled future leap second event
    unsigned short wnt;                             // Current UTC reference week number
    unsigned short wnlsf;                           // Week number when dtlsf becomes effective
    unsigned int tot;                             // Reference time for A0 & A1, sec of GPS week
    double A0,A1;                           // Coeffs for determining UTC time
}TGPSUTCTIMEPARASTRUCT;



/*******************************************************
*         函数声明                                     *
*******************************************************/
extern void GpsTimeToUtcTime( TGPSUTCTIMEPARASTRUCT *ptUtcTimePara,unsigned short usGpsWeekIn, double dGpsSecIn, unsigned short *pusYear, unsigned short *pusMonth, unsigned short *pusDay, unsigned short *pusHour, unsigned short *pusMinute, double *pdSecond );
extern void BdTimeToUtcTime( TBDUTCTIMEPARASTRUCT *ptUtcTimePara,unsigned short usBdWeekIn,double dBdSecIn, unsigned short *pusYear, unsigned short *pusMonth, unsigned short *pusDay, unsigned short *pusHour, unsigned short *pusMinute, double *pdSecond );
extern void UtcToGpsTime( TGPSUTCTIMEPARASTRUCT *ptGpsUtcTime, unsigned short usYear, unsigned short usMonth, unsigned short usDay, unsigned short usHour, unsigned short usMinute, double dSec, unsigned short *pusGpsWeek, double *pdGpsSecond );
extern void UtcToBdTime( TBDUTCTIMEPARASTRUCT *ionoutc, unsigned short y, unsigned short m, unsigned short d, unsigned short hh, unsigned short mm, double ss, unsigned short *BD2Week, double *BD2Second );


#endif

