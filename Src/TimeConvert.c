//#include "stdafx.h"
#include "TimeConvert.h"
#include <math.h>
#include "Global.h"
#include "MyConst.h"
 
void 	GpsTimeToUtcTime( TGPSUTCTIMEPARASTRUCT *ptUtcTimePara,unsigned short usGpsWeekIn, double dGpsSecIn, unsigned short *pusYear, unsigned short *pusMonth, unsigned short *pusDay, unsigned short *pusHour, unsigned short *pusMinute, double *pdSecond ); 
void 	BdTimeToUtcTime( TBDUTCTIMEPARASTRUCT *ptUtcTimePara,unsigned short usBdWeekIn,double dBdSecIn, unsigned short *pusYear, unsigned short *pusMonth, unsigned short *pusDay, unsigned short *pusHour, unsigned short *pusMinute, double *pdSecond ); 
void 	MJDtoYMD( int MJD, unsigned short *pusYear, unsigned short *pusMonth, unsigned short *pusDay );
int 	MJC( int MJD, unsigned short usYear );
void 	GetNumberOfDaysInMonth( unsigned short usYear, unsigned short usMonth,unsigned short* pusDaysInMonth);
int 	LeapYearJudge(unsigned short usYear );
void 	UtcToGpsTime( TGPSUTCTIMEPARASTRUCT *ptGpsUtcTime, unsigned short usYear, unsigned short usMonth, unsigned short usDay, unsigned short usHour, unsigned short usMinute, double dSec, unsigned short *pusGpsWeek, double *pdGpsSecond );
void 	UtcToBdTime( TBDUTCTIMEPARASTRUCT *ionoutc, unsigned short y, unsigned short m, unsigned short d, unsigned short hh, unsigned short mm, double ss, unsigned short *BD2Week, double *BD2Second );
int 	MJD( unsigned short usYear, unsigned short usMonth, unsigned short usDay );

//////////////////////////////////////////////////////////////////////////
///////////////////////////////GPS时转UTC时///////////////////////////////
//////////////////////////////////////////////////////////////////////////  
#pragma CODE_SECTION(GpsTimeToUtcTime,"sect_ECODE_II");  
void GpsTimeToUtcTime( TGPSUTCTIMEPARASTRUCT *ptUtcTimePara,unsigned short usGpsWeekIn, double dGpsSecIn, unsigned short *pusYear, unsigned short *pusMonth, unsigned short *pusDay, unsigned short *pusHour, unsigned short *pusMinute, double *pdSecond )
{
	unsigned short 	usGpsWeek,usYear,usMon,usDay,usHour,usMin,usDaysInMonth = 0;	
	double  		dTotalSeconds,dTimeToLeapSecEvent,dSecNumber,dSecOfDay,dLeapSecDay,dTuct,dGpsSec,dSec,dValue;
	int  			iProirLeapSeconds,iUtcDayNumber,iModifieDay,iUtcRefWeek,iLeapSecEventWeek,iIntegerSecond;

		
	dGpsSec 			= dGpsSecIn;	
	usGpsWeek 			= usGpsWeekIn;
	dSec				= 0.0;

	if( dGpsSec<0.0 )
	{
		dGpsSec 				= dGpsSec + SECONDS_IN_WEEK;
		usGpsWeek--;
	}	
	if( dGpsSec>=SECONDS_IN_WEEK )
	{
		dGpsSec 				= dGpsSec - SECONDS_IN_WEEK;
		usGpsWeek++;
	}	
	if( ptUtcTimePara->vflg )
	{  		
		iUtcRefWeek 			= (short)(usGpsWeek + (short)(ptUtcTimePara->wnt%1024 - ((usGpsWeek%1024)&0xFF)));
		
		if( (usGpsWeek-iUtcRefWeek)>127 ) 
		{
			iUtcRefWeek			+= 256;
		}
		else if( (usGpsWeek-iUtcRefWeek)<-127 )
		{			 
			iUtcRefWeek			-= 256;
		}

		// 新的闰秒生效的周计数；	
		iLeapSecEventWeek = (short)( usGpsWeek + (short)( ptUtcTimePara->wnlsf%1024 - ( (usGpsWeek%1024)&0xFF ) ));

		if( (usGpsWeek-iLeapSecEventWeek)>127) 
		{
			iLeapSecEventWeek 	+= 256;
		}
		else if( (usGpsWeek-iLeapSecEventWeek)<-127 )
		{			
			iLeapSecEventWeek 	-= 256;
		}
		
		// 调整闰秒的时刻	
		dTimeToLeapSecEvent 	= SECONDS_IN_WEEK*( usGpsWeek-iLeapSecEventWeek ) + dGpsSec - ptUtcTimePara->dn*SECONDS_IN_DAY;
		//6hour		
		if( dTimeToLeapSecEvent>(SECONDS_IN_DAY/4.0) )   // case 3
		{
			iProirLeapSeconds 	= ptUtcTimePara->dtlsf;
		}
		else   //case 1,and 2,
		{
			iProirLeapSeconds 	= ptUtcTimePara->dtls;
		}
				
		dTotalSeconds 			= SECONDS_IN_WEEK*(usGpsWeek-iUtcRefWeek) + dGpsSec - ptUtcTimePara->tot;
		dTuct 					= ptUtcTimePara->A0 + ptUtcTimePara->A1*dTotalSeconds;
		dSecNumber 				= dGpsSec - dTuct - iProirLeapSeconds;

		if(dSecNumber<0.0)
		{
			dSecNumber 			= dSecNumber +  SECONDS_IN_WEEK;
			usGpsWeek--;
		}
		if(dSecNumber>=SECONDS_IN_WEEK)
		{
			dSecNumber 			= dSecNumber - SECONDS_IN_WEEK;
			usGpsWeek++;
		}
		
		iUtcDayNumber 			= usGpsWeek*7 + (int)(dSecNumber/SECONDS_IN_DAY); 
		dSecOfDay 				= fmod(dSecNumber,SECONDS_IN_DAY); 
			
		if( (fabs(dTimeToLeapSecEvent))<=(SECONDS_IN_DAY/4.0) )
		{
			dLeapSecDay 		= SECONDS_IN_DAY	+ (ptUtcTimePara->dtlsf - ptUtcTimePara->dtls);
			
			if( dSecOfDay<SECONDS_IN_DAY/2.0 )     
			{
				dSecOfDay 		= dSecOfDay + SECONDS_IN_DAY;
				iUtcDayNumber--; 
			}
			
			if( dSecOfDay>dLeapSecDay )
			{
				iUtcDayNumber++;
				dSecOfDay 		= dSecOfDay -  dLeapSecDay;
			} 
		}
	}
	else 
	{
		//	dGpsSec-=(14.999999996670825);
		iUtcDayNumber 			= usGpsWeek*7 + (int)(dGpsSec/SECONDS_IN_DAY); 
		dSecOfDay 				= fmod(dGpsSec,SECONDS_IN_DAY); 
    }

	iModifieDay 				= 44244 + iUtcDayNumber; 
	MJDtoYMD( iModifieDay, &usYear, &usMon, &usDay );
		
	iIntegerSecond 				= (int)dSecOfDay; 
	usHour 						= (unsigned short)(iIntegerSecond/SECONDS_IN_HOUR);
	if((usHour)>23) 
	{
		usHour 					= 23;
	}

	usMin 						= (unsigned short)((iIntegerSecond- SECONDS_IN_HOUR*(usHour))/60);
	if((usMin)>59) 
	{
		usMin 					= 59;
	}

	dSec 						= dSecOfDay - SECONDS_IN_HOUR*usHour - 60.0*usMin;

	dValue 						= fmod(dSec,1.0);	
	dValue						= fabs(1.0-dValue);
	if(dValue>0.999||dValue < 0.0001)
	{
		//dSec					= round(dSec);
		dSec					= floor(dSec+0.5);
	}

	if( fabs(60.0-dSec)<=0.0001)
	{		 
		dSec			=0.0;
		usMin++;
		if( usMin >= 60 )
		{
		   usMin 		-= 60;
		   usHour++;
		   if( usHour >= 24 )
		   {
			   usHour 	-= 24;
			   usDay++;				   
			   GetNumberOfDaysInMonth( usYear, usMon, &usDaysInMonth);				         
			   if( usDay > usDaysInMonth )
			   {
				   usDay = 1;
				   usMon++;
				   if( usMon > 12 )
				   {
					   usMon = 1;
					   usYear++;
				   }
			   }
		   }
		}
	} 
	    	
	   *pusYear   		= usYear;
	   *pusMonth  		= usMon;
	   *pusDay    		= usDay;
	   *pusHour   		= usHour;
	   *pusMinute 		= usMin;
	   *pdSecond 		= dSec;	
	
}

//////////////////////////////////////////////////////////////////////////
/////////////////////////////北斗时转UTC时////////////////////////////////
//////////////////////////////////////////////////////////////////////////  
#pragma CODE_SECTION(BdTimeToUtcTime,"sect_ECODE_II");  
void BdTimeToUtcTime( TBDUTCTIMEPARASTRUCT *ptUtcTimePara,unsigned short usBdWeekIn,double dBdSecIn, unsigned short *pusYear, unsigned short *pusMonth, unsigned short *pusDay, unsigned short *pusHour, unsigned short *pusMinute, double *pdSecond )
{
	unsigned short 	usBdWeek,usYear,usMon,usDay,usHour,usMin,usDaysInMonth = 0;	
	double  		dTimeToLeapSecEvent,dSecNumber,dSecOfDay,dLeapSecDay,dTuct,dGpsSec,dSec,dValue;
	int  			iProirLeapSeconds,iUtcDayNumber,iModifieDay,iIntegerSecond;

	usBdWeek 			= usBdWeekIn;
	dGpsSec 			= dBdSecIn;
	dSec				= 0.0;
		
	if( dGpsSec<0.0 )
	{
		dGpsSec 				= dGpsSec + SECONDS_IN_WEEK;
		usBdWeek--;
	}	
	if( dGpsSec>=SECONDS_IN_WEEK )
	{
		dGpsSec 				= dGpsSec - SECONDS_IN_WEEK;
		usBdWeek++;
	}	
	if( ptUtcTimePara->vflg )
	{  		
		
		dTimeToLeapSecEvent 	=  dGpsSec - ptUtcTimePara->dn*SECONDS_IN_DAY;
		//6hour		
		if( dTimeToLeapSecEvent>(SECONDS_IN_DAY/4.0) )   // case 3
			iProirLeapSeconds 	= ptUtcTimePara->dtlsf;
		else   //case 1,and 2,
			iProirLeapSeconds 	=  ptUtcTimePara->dtls;		
		
		dTuct 					= ptUtcTimePara->A0 + ptUtcTimePara->A1*dGpsSec;

		dSecNumber 				= dGpsSec - dTuct - iProirLeapSeconds;
		if(dSecNumber<0.0)
		{
			dSecNumber 			= dSecNumber +  SECONDS_IN_WEEK;
			usBdWeek--;
		}
		if(dSecNumber>=SECONDS_IN_WEEK)
		{
			dSecNumber 			= dSecNumber - SECONDS_IN_WEEK;
			usBdWeek++;
		}
		
		iUtcDayNumber 			= usBdWeek*7 + (int)(dSecNumber/SECONDS_IN_DAY); 
		dSecOfDay 				= fmod(dSecNumber,SECONDS_IN_DAY); 	
		if( (fabs(dTimeToLeapSecEvent))<=(SECONDS_IN_DAY/4.0) )
		{
			dLeapSecDay 		= SECONDS_IN_DAY	+ (ptUtcTimePara->dtlsf - ptUtcTimePara->dtls);
			
			if( dSecOfDay<SECONDS_IN_DAY/2.0 )     
			{
				dSecOfDay 		= dSecOfDay + SECONDS_IN_DAY;
				iUtcDayNumber--; 
			}
			
			if( dSecOfDay>dLeapSecDay )
			{				
				dSecOfDay 		= dSecOfDay -  dLeapSecDay;
				iUtcDayNumber++;
			} 
		}
	}
	else 
	{
		
		iUtcDayNumber 			= usBdWeek*7 + (int)(dGpsSec/SECONDS_IN_DAY); 
		dSecOfDay 				= fmod(dGpsSec,SECONDS_IN_DAY); 
    }
	iModifieDay 				= 53736 + iUtcDayNumber; 
	MJDtoYMD( iModifieDay, &usYear, &usMon, &usDay );
		
	iIntegerSecond 				= (int)dSecOfDay; 

	usHour 						= (unsigned short)(iIntegerSecond/SECONDS_IN_HOUR);
	if((usHour)>23)
	{ 
		usHour 					= 23;
	}

	usMin 						= (unsigned short)((iIntegerSecond- SECONDS_IN_HOUR*(usHour))/60);
	if((usMin)>59) 
	{
		usMin 					= 59;
	}

	dSec 						= dSecOfDay - SECONDS_IN_HOUR*usHour - 60.0*usMin;

    dValue 						= fmod(dSec,1.0);	
	dValue						= fabs(1.0-dValue);
	if(dValue>0.99999999||dValue < 0.00000001)
	{
		//dSec					= round(dSec);
	}
	if( fabs(60.0-dSec)<=0.0001)
	{		 
	   dSec						=0.0;
	   usMin++;
	   if( usMin >= 60 )
	   {
		   usMin 				-= 60;
		   usHour++;
		   if( usHour >= 24 )
		   {
			   usHour 			-= 24;
			   usDay++;				   
			   GetNumberOfDaysInMonth( usYear, usMon, &usDaysInMonth);				         
			   if( usDay > usDaysInMonth )
			   {
				   usDay 		= 1;
				   usMon++;
				   if( usMon > 12 )
				   {
					   usMon	= 1;
					   usYear++;
				   }
			   }
		   }
	   }
	} 
	    	
	*pusYear   					= usYear;
	*pusMonth  					= usMon;
	*pusDay    					= usDay;
	*pusHour   					= usHour;
	*pusMinute 					= usMin;
	*pdSecond 					= dSec;	
}

//////////////////////////////////////////////////////////////////////////
//////////////////////// 儒略日转为年、月、日/////////////////////////////
//////////////////////////////////////////////////////////////////////////  
#pragma CODE_SECTION(MJDtoYMD,"sect_ECODE_II");  
void MJDtoYMD( int MJD, unsigned short *pusYear, unsigned short *pusMonth, unsigned short *pusDay )
{
    unsigned short  usYear,usMonth,usDay,usDaysInMonth,usDaysInYear;
    int 			iDaysInYear,PreNineMonths,TwelveMonths;
    
    // 儒略日转为年    
    usYear 				= (unsigned short)( (MJD-15019L) * 4 / 1461 );
	// 当前确切的年技术
    usYear 				+= 1900;    
    usDaysInYear 		= 365;
   
    iDaysInYear = MJC( MJD, usYear );
	
	// 计算年内天
    if( iDaysInYear<0 )
    {  
        usYear --;    
    }

    if( usYear==4 * ( usYear / 4 ) )
    {
        usDaysInYear	=366;    
    }
	
	// 计算年内天
    iDaysInYear 		= MJC( MJD, usYear );
    if( iDaysInYear == usDaysInYear )
    {
        usYear++;
    }
    if( usYear==( 4 * ( usYear / 4 ) ) )
    {
        usDaysInMonth 	= 29;    
    }

    iDaysInYear = MJC( MJD, usYear );
	// 计算年内月
    usMonth = (unsigned short)( ( iDaysInYear / 31 ) + 1 ); 
       
    if( usMonth==2 )
    {
        usDaysInMonth 	= 28;
    }
    else if( (usMonth==4)||(usMonth==6)||( usMonth==9)||(usMonth==11) ) 
    {        
        usDaysInMonth 	= 30;
    }
    else if( (usMonth==1)||(usMonth==3)||(usMonth==5)||(usMonth==7)||(usMonth==8)||(usMonth==10)||(usMonth==12) )
    {
        usDaysInMonth 	= 31;
    } 
               
    PreNineMonths 					= 275L * usMonth / 9;
    TwelveMonths 					= (usMonth + 9) / 12;

    // 计算月内日
    usDay 							= (unsigned short)( MJD + 678987L -367L * usYear - PreNineMonths +( 7L * ( usYear + TwelveMonths ) / 4 ) );
    if( usDay>usDaysInMonth ) 
    {
        usMonth 		+= 1;
        usDay 			= (unsigned short)(usDay-usDaysInMonth);
    }            
    *pusYear 			= usYear;
    *pusMonth 			= usMonth;
    *pusDay 			= usDay;            
}

//////////////////////////////////////////////////////////////////////////
//////////////////// 儒略日转为当前年对应的年内天数 //////////////////////
////////////////////////////////////////////////////////////////////////// 
#pragma CODE_SECTION(MJC,"sect_ECODE_II");
int MJC( int MJD, unsigned short usYear )
{
    int iTemp1, iTemp2, iDaysInYear; 
       
    iTemp1 			= 7L * usYear / 4;
    iTemp2 			= 367L * usYear;
    iDaysInYear 	= MJD - iTemp2 + iTemp1 + 678956L; 
       
    return iDaysInYear;
}

//////////////////////////////////////////////////////////////////////////
///////////////////////// 得到当前月对应的天数 ///////////////////////////
//////////////////////////////////////////////////////////////////////////  
#pragma CODE_SECTION(GetNumberOfDaysInMonth,"sect_ECODE_III"); 
void GetNumberOfDaysInMonth( unsigned short usYear, unsigned short usMonth,unsigned short* pusDaysInMonth)
{
	int 			iLeapYearFlag;
	unsigned short 	usDays = 0;	

	iLeapYearFlag = LeapYearJudge( usYear );
		
	switch(usMonth)
	{
	case  1: 
		usDays	= 31;
		break;
	case  2: 
		usDays 	= (1==iLeapYearFlag)? 29 : 28;    
	case  3: 
		usDays 	= 31;
		break;
	case  4:
		usDays 	= 30;
		break;
	case  5: 
		usDays 	= 31;
		break;
	case  6:
		usDays 	= 30;
		break;
	case  7:
		usDays 	= 31;
		break;
	case  8: 
		usDays 	= 31; 
		break;
	case  9: 
		usDays 	= 30; 
		break;
	case 10:
		usDays 	= 31; 
		break;
	case 11: 
		usDays 	= 30;
		break;
	case 12: 
		usDays 	= 31;
		break;
	default: 
		usDays	=30;
		break; 	   
		
	}	
	*pusDaysInMonth = usDays;	
} 

//////////////////////////////////////////////////////////////////////////
//////////////////////////////// 闰年判断 ////////////////////////////////
//////////////////////////////////////////////////////////////////////////  
#pragma CODE_SECTION(LeapYearJudge,"sect_ECODE_III");
int LeapYearJudge(unsigned short usYear )
{
	int iLeapYearFlag = 0;
		
	if( (usYear%4) == 0 )
	{
		iLeapYearFlag = 1;

		if( (usYear%100) == 0 )
		{
			if( (usYear%400) == 0 )
			{
				iLeapYearFlag = 1;
			}
			else
			{
				iLeapYearFlag = 0;
			}
		}
	}

	return iLeapYearFlag;
} 

//////////////////////////////////////////////////////////////////////////
/////////////////////////////UTC转化为GPS时///////////////////////////////
//////////////////////////////////////////////////////////////////////////  
#pragma CODE_SECTION(UtcToGpsTime,"sect_ECODE_II"); 
void UtcToGpsTime( TGPSUTCTIMEPARASTRUCT *ptGpsUtcTime, unsigned short usYear, unsigned short usMonth, unsigned short usDay, unsigned short usHour, unsigned short usMinute, double dSec, unsigned short *pusGpsWeek, double *pdGpsSecond )
{
    unsigned int 	uiSecOfUtcDay;
    int 			iDays;
    short 			sUtcRefWeek, sLeapSecEventWeek, sPriorLeapSecs, iTodaysLeapSecs, sGpsWeek,sGpsWeekTemp;   
    double 			dGpsSecond,dGpsSecondTemp,dSecond,dTimeToLeapSecondEvent;

    
	// 计算天内秒
    uiSecOfUtcDay			= (unsigned int )( SECONDS_IN_HOUR * usHour + 60.0 * usMinute + (floor(dSec)) );
	// 年、月、日转为儒略日
    iDays 					= MJD( usYear, usMonth, usDay ) - 44244L;  
    // 粗略计算GPS周 
    sGpsWeek 				= (short)(iDays/7L);
	// 粗略计算GPS秒
    dGpsSecond 				= SECONDS_IN_DAY * (iDays%7) + (SECONDS_IN_HOUR*usHour) + (60.0 * usMinute) + dSec;
    if( dGpsSecond<0.0 )
    {
        dGpsSecond += SECONDS_IN_WEEK;
        sGpsWeek--;
    }
    
    if( dGpsSecond>=SECONDS_IN_WEEK )
    {
        dGpsSecond -= SECONDS_IN_WEEK;
        sGpsWeek++;
    } 
       
    if(1 == ptGpsUtcTime->vflg)
    {        
        sUtcRefWeek 			= (short)( sGpsWeek + (short)( ( ptGpsUtcTime->wnt % 1024 ) - ( ( sGpsWeek % 1024 ) & 0xFF ) ) );
        if( (sGpsWeek-sUtcRefWeek)>127 )
        {
            sUtcRefWeek 		+= 256;
        }
        else if( (sGpsWeek-sUtcRefWeek)<-127)
        {

            sUtcRefWeek 		-= 256;

        }
        
        sLeapSecEventWeek = (short)( sGpsWeek + (short)( ( ptGpsUtcTime->wnlsf % 1024 ) - ( ( sGpsWeek % 1024 ) & 0xFF ) ) );
        if( ( sGpsWeek - sLeapSecEventWeek ) > 127 )
        {
            sLeapSecEventWeek 	+= 256;
        }
        else if( (sGpsWeek-sLeapSecEventWeek)<-127 )
        {
            sLeapSecEventWeek 	-= 256;
        }   
             
        dSecond 				= SECONDS_IN_WEEK * ( sGpsWeek - sUtcRefWeek ) + (dGpsSecond - ptGpsUtcTime->tot);

        dGpsSecond 				+= ptGpsUtcTime->A0 + ptGpsUtcTime->A1 * dSecond ; 
            
        sGpsWeekTemp 			= sGpsWeek;
        dGpsSecondTemp 			= dGpsSecond + ptGpsUtcTime->dtls;

        if( dGpsSecondTemp>=SECONDS_IN_WEEK )
        {
            dGpsSecondTemp 		-= SECONDS_IN_WEEK;
            sGpsWeekTemp 		+= 1;
        }
        else 
        {
            if( dGpsSecondTemp<0.0 )
            {
                dGpsSecondTemp 	+= SECONDS_IN_WEEK;
                sGpsWeekTemp 	-= 1;
            }
        }  
              
        dTimeToLeapSecondEvent 	= SECONDS_IN_WEEK * (sLeapSecEventWeek-sGpsWeekTemp) + (ptGpsUtcTime->dn * 86400UL) - dGpsSecondTemp;
        if( (double)dTimeToLeapSecondEvent>21600.0 )
        {
            iTodaysLeapSecs 	= 0;
            sPriorLeapSecs 		= (short)ptGpsUtcTime->dtls;
        }
        else if((double)dTimeToLeapSecondEvent<-21600.0)
        {
            iTodaysLeapSecs 	= 0;
            sPriorLeapSecs 		= (short)ptGpsUtcTime->dtlsf;
        }
        else
        {
            iTodaysLeapSecs 	= (short)(ptGpsUtcTime->dtlsf - ptGpsUtcTime->dtls);
            sPriorLeapSecs 		= (short)ptGpsUtcTime->dtls;//???
        }
        
    }
    else  
    {
        sPriorLeapSecs 			= 0;
        iTodaysLeapSecs 		= 0;
    }
    
    dGpsSecond 					= dGpsSecond + sPriorLeapSecs;
    if( (iTodaysLeapSecs)&&((double)uiSecOfUtcDay<21600) )
    {
        dGpsSecond 				= dGpsSecond + iTodaysLeapSecs;
    }

    if( dGpsSecond>SECONDS_IN_WEEK )
    {
        dGpsSecond 				-= SECONDS_IN_WEEK;
        sGpsWeek++;
    }
    else 
    {
        if( dGpsSecond<0.0 )
        {
            dGpsSecond 			+= SECONDS_IN_WEEK;
            sGpsWeek--;
        }
    }    
    *pusGpsWeek 				=(unsigned short) sGpsWeek;
    *pdGpsSecond 				= dGpsSecond;
}

//////////////////////////////////////////////////////////////////////////
/////////////////////////////UTC转化为北斗时//////////////////////////////
//////////////////////////////////////////////////////////////////////////  
#pragma CODE_SECTION(UtcToBdTime,"sect_ECODE_II");  
void UtcToBdTime( TBDUTCTIMEPARASTRUCT *ptBdUtcTime, unsigned short usYear, unsigned short usMonth, unsigned short usDay, unsigned short usHour, unsigned short usMinute, double dSec, unsigned short *pusBdWeek, double *pdBdSecond )
{
    unsigned int  	uiSecNumOfUTCDay;
    int 			iDays;
    short 			sPriorLeapSecs,sTodaysLeapSecs,sBdWeek,sBdWeekTemp;   
    double 			dBdSecond,dBdSecondTemp,dTimeToLeapSecondEvent;

	// 计算天内秒
    uiSecNumOfUTCDay			= (unsigned int )( SECONDS_IN_HOUR * usHour + 60.0 * usMinute + (double)(floor(dSec)) );
	// 年、月、日转为儒略日
    iDays 						= MJD( usYear, usMonth, usDay ) - 53736L; 
    // 粗略计算BD周   
    sBdWeek 					= (short)( iDays / 7L );
	// 粗略计算BD秒 
    dBdSecond 					= SECONDS_IN_DAY * (iDays%7) + (SECONDS_IN_HOUR * usHour ) + ( 60.0 * usMinute ) + dSec;
    if( dBdSecond<0.0 )
    {
        dBdSecond 				+= SECONDS_IN_WEEK;
        sBdWeek--;
    }    
    if( dBdSecond>=SECONDS_IN_WEEK )
    {
        dBdSecond 				-= SECONDS_IN_WEEK;
        sBdWeek++;
    } 
       
    if(1 == ptBdUtcTime->vflg)
    { 
        dBdSecond 				+= ptBdUtcTime->A0 + ptBdUtcTime->A1*dBdSecond ;     
        sBdWeekTemp 			= sBdWeek;
        dBdSecondTemp 			= dBdSecond + ptBdUtcTime->dtls;
        if( dBdSecondTemp >= SECONDS_IN_WEEK )
        {
            dBdSecondTemp 		-= SECONDS_IN_WEEK;
            sBdWeekTemp 		+= 1;
        }
        else if( dBdSecondTemp<0.0 )
        {
            dBdSecondTemp 		+= SECONDS_IN_WEEK;
            sBdWeekTemp 		-= 1;
        } 
               
        dTimeToLeapSecondEvent 	= ptBdUtcTime->dn * SECONDS_IN_DAY - dBdSecondTemp;
        if( (double)dTimeToLeapSecondEvent>21600.0 )
        {
            sTodaysLeapSecs 	= 0;
            sPriorLeapSecs 		= (short)ptBdUtcTime->dtls;
        }
        else if((double)dTimeToLeapSecondEvent<-21600.0)
        {
            sTodaysLeapSecs 	= 0;
            sPriorLeapSecs 		= (short)ptBdUtcTime->dtlsf;
        }    
        else
        {
            sTodaysLeapSecs 	= (short)(ptBdUtcTime->dtlsf - ptBdUtcTime->dtls);
            sPriorLeapSecs 		= (short)ptBdUtcTime->dtls;
        }
        
    }
    else  
    {
        sPriorLeapSecs 			= 0;
        sTodaysLeapSecs 		= 0;
    }
    
    dBdSecond 					= dBdSecond + sPriorLeapSecs;

    if( (sTodaysLeapSecs)&&(uiSecNumOfUTCDay<21600) )
    {
        dBdSecond 				= dBdSecond + sTodaysLeapSecs;
    }

    if( dBdSecond>SECONDS_IN_WEEK )
    {
        dBdSecond 				-= SECONDS_IN_WEEK;
        sBdWeek++;
    }
    else  if( dBdSecond<0.0 )
    {
	    dBdSecond 				+= SECONDS_IN_WEEK;
	    sBdWeek--;

    }    
    *pusBdWeek 					= (unsigned short) sBdWeek;
    *pdBdSecond 				= dBdSecond;
}

//////////////////////////////////////////////////////////////////////////
/////////////////////把年、月、日对应为约化的儒略日///////////////////////
////////////////////////////////////////////////////////////////////////// 	
#pragma CODE_SECTION(MJD,"sect_ECODE_II");  		 
int MJD( unsigned short usYear, unsigned short usMonth, unsigned short usDay )
{
    unsigned short 	m1,m2,m3;
    int 			YMJD = 0;

    m1 				= ( usMonth + 9 ) / 12;
    m2 				= 7 * ( usYear + m1 ) / 4;
    m3 				= ( 275 * usMonth ) / 9;
    YMJD 			= 367L * usYear - 678987L - m2 + m3 + usDay;    
    return YMJD;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void UtcParaInit()
{
	
		g_tGpsUtcTimePara.A0		= 0;
		g_tGpsUtcTimePara.A1		= 0;
		g_tGpsUtcTimePara.dtls		= 17;
		g_tGpsUtcTimePara.tot		= 0;
		g_tGpsUtcTimePara.wnt		= 0;
		g_tGpsUtcTimePara.wnlsf		= 0;
		g_tGpsUtcTimePara.dn		= 0;
		g_tGpsUtcTimePara.dtlsf		= 17;
		g_tGpsUtcTimePara.vflg		= 1;


		g_tBdUtcTimePara.A0			= 0;
		g_tBdUtcTimePara.A1			= 0;
		g_tBdUtcTimePara.dtls		= 2;
		g_tBdUtcTimePara.wnlsf		= 0;
		g_tBdUtcTimePara.dn			= 0;
		g_tBdUtcTimePara.dtlsf		= 2;
		g_tBdUtcTimePara.vflg		= 1;

}
