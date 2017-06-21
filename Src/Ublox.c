/*------------------------------------------------------------------------------
* ublox.c : ublox receiver dependent functions
*
*          Copyright (C) 2007-2013 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] ublox-AG, GPS.G3-X-03002-D, ANTARIS Positioning Engine NMEA and UBX
*         Protocol Specification, Version 5.00, 2003
*
* version : $Revision: 1.2 $ $Date: 2008/07/14 00:05:05 $
* history : 2007/10/08 1.0 new
*           2008/06/16 1.1 separate common functions to rcvcmn.c
*           2009/04/01 1.2 add range check of prn number
*           2009/04/10 1.3 refactored
*           2009/09/25 1.4 add function gen_ubx()
*           2010/01/17 1.5 add time tag adjustment option -tadj sec
*           2010/10/31 1.6 fix bug on playback disabled for raw data (2.4.0_p9)
*           2011/05/27 1.7 add almanac decoding
*                          add -EPHALL option
*                          fix problem with ARM compiler
*           2013/02/23 1.8 fix memory access violation problem on arm
*                          change options -tadj to -TADJ, -invcp to -INVCP
*-----------------------------------------------------------------------------*/
//#include "stdafx.h" 
#include "rtklib.h"
#include <math.h>
#include "Global.h"
#include <string.h>

extern nav_t nav;

#define UBXSYNC1    0xB5        /* ubx message sync code 1 */
#define UBXSYNC2    0x62        /* ubx message sync code 2 */
#define UBXCFG      0x06        /* ubx message cfg-??? */

#define ID_NAVSOL   0x0106      /* ubx message id: nav solution info */
#define ID_NAVTIME  0x0120      /* ubx message id: nav time gps */
#define ID_RXMRAW   0x0210      /* ubx message id: raw measurement data */
#define ID_RXMSFRB  0x0211      /* ubx message id: subframe buffer */
#define ID_RXMSFRBX 0x0213      /* ubx message id: raw subframe data */
#define ID_RXMRAWX  0x0215      /* ubx message id: multi-gnss raw meas data */
#define ID_TRKD5    0x030A      /* ubx message id: trace mesurement data */
#define ID_TRKMEAS  0x0310      /* ubx message id: trace mesurement data */
#define ID_TRKSFRBX 0x030F      /* ubx message id: trace subframe buffer */
#define ID_POSECEF  0x0101      /* ubx message id: Position Solution in ECEF */
#define ID_POSBASE  0x0111
#define ID_VIEW_RTK 0x0118

#define FU1         1           /* ubx message field types */
#define FU2         2
#define FU4         3
#define FI1         4
#define FI2         5
#define FI4         6
#define FR4         7
#define FR8         8
#define FS32        9

#define ROUND(x)    (int)floor((x)+0.5)

static const char rcsid[]="$Id: ublox.c,v 1.2 2008/07/14 00:05:05 TTAKA Exp $";

void *memcpy1(void *desc,const void * src,size_t size)
{
	if((desc == NULL) && (src == NULL))
 {
  return NULL;
 }
 {   
    unsigned char *desc1 = (unsigned char*)desc;
    unsigned char *src1 = (unsigned char*)src;
    while(size-- >0)
    {
     *desc1 = *src1;
     desc1++;
     src1++;
    }
 }
 return desc;
}
/* get fields (little-endian) ------------------------------------------------*/
#define U1(p) (*((unsigned char *)(p)))
#define I1(p) (*((char *)(p)))
static unsigned short U2(unsigned char *p) 
{
	unsigned short u; 
	memcpy1(&u,p,2); 
	return u;
}
static unsigned int   U4(unsigned char *p) {unsigned int   u; memcpy1(&u,p,4); return u;}
static int   		  I4(unsigned char *p) {int   u; memcpy1(&u,p,4); return u;}
static float          R44(unsigned char *p) {float          r; memcpy1(&r,p,4); return r;}
static double         R8(unsigned char *p) {double         r; memcpy1(&r,p,8); return r;}

/* set fields (little-endian) ------------------------------------------------*/
 void setU1(unsigned char *p, unsigned char  u) {*p=u;}
 void setU2(unsigned char *p, unsigned short u) {memcpy1(p,&u,2);}
 void setU4(unsigned char *p, unsigned int   u) {memcpy1(p,&u,4);}
 void setI1(unsigned char *p, char           i) {*p=(unsigned char)i;}
 void setI2(unsigned char *p, short          i) {memcpy1(p,&i,2);}
 void setI4(unsigned char *p, int            i) {memcpy1(p,&i,4);}
 void setR4(unsigned char *p, float          r) {memcpy1(p,&r,4);}
 void setR8(unsigned char *p, double         r) {memcpy1(p,&r,8);}


#define P2_66       1.355252715606881E-20 /* 2^-66 for BeiDou ephemeris */

/* get two component bits ----------------------------------------------------*/
static unsigned int getbitu2(const unsigned char *buff, int p1, int l1, int p2,
                             int l2)
{
    return (getbitu(buff,p1,l1)<<l2)+getbitu(buff,p2,l2);
}
static int getbits2(const unsigned char *buff, int p1, int l1, int p2, int l2)
{
    if (getbitu(buff,p1,1))
        return (int)((getbits(buff,p1,l1)<<l2)+getbitu(buff,p2,l2));
    else
        return (int)getbitu2(buff,p1,l1,p2,l2);
}
/* get three component bits --------------------------------------------------*/
static unsigned int getbitu3(const unsigned char *buff, int p1, int l1, int p2,
                             int l2, int p3, int l3)
{
    return (getbitu(buff,p1,l1)<<(l2+l3))+(getbitu(buff,p2,l2)<<l3)+
		getbitu(buff,p3,l3);
}//
static int getbits3(const unsigned char *buff, int p1, int l1, int p2, int l2,
                    int p3, int l3)
{
    if (getbitu(buff,p1,1))
        return (int)((getbits(buff,p1,l1)<<(l2+l3))+
		(getbitu(buff,p2,l2)<<l3)+getbitu(buff,p3,l3));
    else
        return (int)getbitu3(buff,p1,l1,p2,l2,p3,l3);
}
/* merge two components ------------------------------------------------------*/
static unsigned int merge_two_u(unsigned int a, unsigned int b, int n)
{
    return (a<<n)+b;
}
static int merge_two_s(int a, unsigned int b, int n)
{
    return (int)((a<<n)+b);
}
/* get sign-magnitude bits ---------------------------------------------------*/
static double getbitg(const unsigned char *buff, int pos, int len)
{
    double value=getbitu(buff,pos+1,len-1);
    return getbitu(buff,pos,1)?-value:value;
}

/* checksum ------------------------------------------------------------------*/
static int checksum(unsigned char *buff, int len)
{
    unsigned char cka=0,ckb=0;
    int i;
    
    for (i=2;i<len-2;i++) {
        cka+=buff[i]; ckb+=cka;
    }
    return cka==buff[len-2]&&ckb==buff[len-1];
}
static void setcs(unsigned char *buff, int len)
{
    unsigned char cka=0,ckb=0;
    int i;
    
    for (i=2;i<len-2;i++) {
        cka+=buff[i]; ckb+=cka;
    }
    buff[len-2]=cka;
    buff[len-1]=ckb;
}

/* ubx gnss indicator (ref [2] 25) -------------------------------------------*/
static int ubx_sys(int ind)
{
    switch (ind) {
	case 0: return SYS_GPS;
	case 1: return SYS_SBS;
	case 2: return SYS_GAL;
	case 3: return SYS_CMP;
	case 5: return SYS_QZS;
	case 6: return SYS_GLO;
    }
    return 0;
}

/* decode ublox rxm-raw: raw measurement data --------------------------------*/
static int decode_rxmraw(raw_t *raw)
{
    gtime_t time;
    double tow,tt,tadj=0.0,toff=0.0,tn;
    int i,j,prn,sat,n=0,nsat,week;
    unsigned char *p=raw->buff+6;
    char *q,tstr[32];
    
    //trace(4,"decode_rxmraw: len=%d\n",raw->len);
    
    /* time tag adjustment option (-TADJ) */
    if ((q=strstr((char *)raw->opt,"-TADJ"))) {
        sscanf(q,"-TADJ=%lf",&tadj);
    }
    tow =U4(p  );
    week=U2(p+4);
    nsat=U1(p+6);
    if (raw->len<12+24*nsat) {
        //trace(2,"ubx rxmraw length error: len=%d nsat=%d\n",raw->len,nsat);
        return -1;
    }
    time=gpst2time(week,tow*0.001);
    
    /* time tag adjustment */
    if (tadj>0.0) {
        tn=time2gpst(time,&week)/tadj;
        toff=(tn-floor(tn+0.5))*tadj;
        time=timeadd(time,-toff);
    }
    if (fabs(tt=timediff(time,raw->time))<=1e-3) {
        //time2str(time,tstr,3);
        //trace(2,"ubx rxmraw time tag duplicated: time=%s\n",tstr);
        return 0;
    }
    for (i=0,p+=8;i<nsat&&i<MAXOBS;i++,p+=24) {
        raw->obs.data[n].time=time;
        raw->obs.data[n].L[0]  =R8(p   )-toff*FREQ1;
        raw->obs.data[n].P[0]  =R8(p+ 8)-toff*CLIGHT;
        raw->obs.data[n].D[0]  =R44(p+16);
        prn                    =U1(p+20);
        raw->obs.data[n].SNR[0]=(unsigned char)(I1(p+22)*4.0+0.5);
        raw->obs.data[n].LLI[0]=U1(p+23);
        raw->obs.data[n].code[0]=CODE_L1C;
        
        /* phase polarity flip option (-INVCP) */
        if (strstr(raw->opt,"-INVCP")) {
            raw->obs.data[n].L[0]=-raw->obs.data[n].L[0];
        }
        if (!(sat=satno(MINPRNSBS<=prn?SYS_SBS:SYS_GPS,prn))) {
            //trace(2,"ubx rxmraw sat number error: prn=%d\n",prn);
            continue;
        }
        raw->obs.data[n].sat=sat;
        
        if (raw->obs.data[n].LLI[0]&1) raw->lockt[sat-1][0]=0.0;
        else if (tt<0.0||10.0<tt) raw->lockt[sat-1][0]=0.0;
        else raw->lockt[sat-1][0]+=tt;
        
        for (j=1;j<NFREQ;j++) {
            raw->obs.data[n].L[j]=raw->obs.data[n].P[j]=0.0;
            raw->obs.data[n].D[j]=0.0;
            raw->obs.data[n].SNR[j]=raw->obs.data[n].LLI[j]=0;
            raw->obs.data[n].code[j]=CODE_NONE;
        }
        n++;
    }
    raw->time=time;
    raw->obs.n=n;
    return 1;
}

/* decode ubx-rxm-rawx: multi-gnss raw measurement data (ref [3]) ------------*/
static int decode_rxmrawx(raw_t *raw,unsigned short usFlag)
{
    gtime_t time;
    double tow0,tow,cp1,pr1;
    int i,j,sys,prn,sat,fcn,n=0,nsat,week,tstat,lockt,halfc;
    unsigned char *p=raw->buff+6,*q;
    //trace(4,"decode_rxmrawx: len=%d\n",raw->len);
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX RXM-RAWX  (%4d): nsat=%d",raw->len,U1(p+11));
    }
		//printf("nsat = %d\r\n",nsat);
    nsat=U1(p+11);
		//printf("p address = %p\r\n",p);
		//printf("raw->len = %d\r\n",raw->len);
    if (raw->len < (24+32*nsat)) {
        //trace(2,"ubx rxmrawx length error: len=%d nsat=%d\n",raw->len,nsat);
        //LogStr("ubx rxmrawx length error: len=%d nsat=%d\n",raw->len,nsat);	
        return -1;
    }		
	//	printf("tow0 = %lf\r\n",tow0);
    tow0=R8(p);
	//	printf("tow0 = %lf\r\n",tow0);
    week=U2(p+8);
    tow=ROUND(tow0/0.1)*0.1; /* round by 100 ms */
    time=gpst2time(week,tow);
    
    for (i=0,p+=16;i<nsat&&i<MAXOBS;i++,p+=32) {
        
        if (!(sys=ubx_sys(U1(p+20)))) {
            //trace(2,"ubx rxmrawx: system error\n");
            //LogStr("ubx rxmrawx: system error\n");
            continue;
        }
        prn=U1(p+21)+(sys==SYS_QZS?192:0);
        if (!(sat=satno(sys,prn))) {
            //trace(2,"ubx rxmrawx sat number error: sys=%2d prn=%2d\n",sys,prn);
            //LogStr("ubx rxmrawx sat number error: sys=%2d prn=%2d\n",sys,prn);
            continue;
        }
        tstat=U1(p+30); /* tracking status */
        pr1=tstat&1?R8(p)+(tow-tow0)*CLIGHT:0.0;
        cp1=tstat&2?R8(p+8):0.0;
        if (cp1==-0.5) cp1=0.0; /* invalid phase */
        raw->obs.data[n].sat=sat;
        raw->obs.data[n].time=time;
        raw->obs.data[n].P[0]=pr1;
        raw->obs.data[n].L[0]=cp1;
        raw->obs.data[n].D[0]=R44(p+16);
        raw->obs.data[n].SNR[0]=116;//U1(p+26)*4
        raw->obs.data[n].LLI[0]=0;
        raw->obs.data[n].code[0]=sys==SYS_CMP?CODE_L1I:CODE_L1C;
        lockt=U2(p+24);    /* lock time count (ms) */
        halfc=tstat&8?1:0; /* half cycle subtracted from phase */

		 
        if (cp1!=0.0) { /* carrier-phase valid */
            
            /* LLI: bit1=loss-of-lock,bit2=half-cycle-invalid */
            raw->obs.data[n].LLI[0]|=lockt==0||lockt<raw->lockt[sat-1][0]?1:0;
            raw->obs.data[n].LLI[0]|=halfc!=raw->halfc[sat-1][0]?1:0;
            raw->obs.data[n].LLI[0]|=tstat&4?0:2;
            raw->lockt[sat-1][0]=lockt;
            raw->halfc[sat-1][0]=halfc;
        }
        for (j=1;j<NFREQ+NEXOBS;j++) {
            raw->obs.data[n].L[j]=raw->obs.data[n].P[j]=0.0;
            raw->obs.data[n].D[j]=0.0;
            raw->obs.data[n].SNR[j]=raw->obs.data[n].LLI[j]=0;
            raw->obs.data[n].code[j]=CODE_NONE;
        }
        n++;
    }
    raw->time=time;
    raw->obs.n=n;
	
	//Base
	if (usFlag == 0)
	{
		g_usRawBaseFlag = 1;
	}
	else if (usFlag == 1)
	{
		g_usRawRoverFlag = 1;
		//g_uiStartTic			= PRD_getticks();
	}
	
	
    return 1;
}

//extern void DrawSwitchSrceen();
extern unsigned int gCalcRtkUseTime;

static int decode_view_rtk(raw_t *raw)
{
    rtk_t 					*ptRtk = &g_tRtk;
    unsigned char *p=raw->buff+6;
    double 					dRoverSecond;
    int 					iGpsWeek;

    iGpsWeek = (int)U2(p);                  p += 2;
    dRoverSecond = (double) U4(p)*1e-3;     p += 4;

    g_tRawBase.time = gpst2time(iGpsWeek,dRoverSecond);

    ptRtk->rb[0] = (double) R8(p);   p += 8;
    ptRtk->rb[1] = (double) R8(p);   p += 8;
    ptRtk->rb[2] = (double) R8(p);   p += 8;

    g_tRawBase.obs.n = U1(p);        p += 1;

    iGpsWeek = (int)U2(p);                  p += 2;
    dRoverSecond = (double) U4(p)*1e-3;     p += 4;

    ptRtk->sol.time = gpst2time(iGpsWeek,dRoverSecond);

    ptRtk->sol.rr[0] = (double) R8(p);      p += 8;
    ptRtk->sol.rr[1] = (double) R8(p);      p += 8;
    ptRtk->sol.rr[2] = (double) R8(p);      p += 8;
    ptRtk->sol.rr[3] = (double) R8(p);      p += 8;
    ptRtk->sol.rr[4] = (double) R8(p);      p += 8;
    ptRtk->sol.rr[5] = (double) R8(p);      p += 8;

		ptRtk->sol.age = R44(p); 	            p+=4;
		ptRtk->sol.ratio = R44(p); 	            p+=4;    
    
    ptRtk->sol.stat = U1(p);    p += 1;
    g_tRawRover.obs.n = U1(p);  p += 1;
    ptRtk->sol.ns = U1(p);      p += 1;

    gCalcRtkUseTime = U4(p);    p += 4;

    //DrawSwitchSrceen(); 
		return 1;
}

/* decode ublox POSECEF: Position Solution in ECEF --------------------------------*/
static int decode_rxposecef(raw_t *raw)
{
    gtime_t time;
	double dPosAcc;
    unsigned char *p=raw->buff+6;
	char *q;
    double tow,dPos[3];
	static unsigned char ucFilterCnt = 0; 
	
    
    //trace(4,"decode_rxmraw: len=%d\n",raw->len);
    
    
    tow  = U4(p  );
    //dPos[0] = (double)getbits(p, 32, 32)*0.01;
    //dPos[1] = (double)getbits(p, 64, 32)*0.01;
	//dPos[2] = (double)getbits(p, 96, 32)*0.01;
	
	dPos[0] = (int)U4(p+4)*0.01;
    dPos[1] = (int)U4(p+8)*0.01;
	dPos[2] = (int)U4(p+12)*0.01;
	
	dPosAcc = (int)U4(p+16)*0.01;;

	if((dPosAcc<3.0))//&& (ucFilterCnt<20))
	{
		//if(ucFilterCnt==0)
		{
			g_tBasePose.dFilterX = dPos[0];
			g_tBasePose.dFilterY = dPos[1];
			g_tBasePose.dFilterZ = dPos[2];	
      g_tBasePose.uiPosFilterFlag = 1;
            		            
		}
		/*else
		{
			g_tBasePose.dFilterX = (g_tBasePose.dFilterX*ucFilterCnt + dPos[0])/(ucFilterCnt+1);
			g_tBasePose.dFilterY = (g_tBasePose.dFilterY*ucFilterCnt + dPos[1])/(ucFilterCnt+1);
			g_tBasePose.dFilterZ = (g_tBasePose.dFilterZ*ucFilterCnt + dPos[2])/(ucFilterCnt+1);
		}
		ucFilterCnt++;
		
		if(ucFilterCnt==20)
		{
			g_tBasePose.uiPosFilterFlag = 1;
            ucFilterCnt=1;
		}*/
	}
    else if (dPosAcc > 6.0)
    {
        g_tBasePose.uiPosFilterFlag = 0;
    }
         
    return 1;
}


static int decode_base_pos(raw_t *raw)
{
    unsigned char *p=raw->buff+6;	

	g_tReadPosFromBase.dUserSetX = R8(p); p += 8;
	g_tReadPosFromBase.dUserSetY = R8(p); p += 8;
	g_tReadPosFromBase.dUserSetZ = R8(p); p += 8;
	
	g_tBasePose.uiUserSetFlag = 1;
        
    return 1;
}
/* save subframe -------------------------------------------------------------*/
static int save_subfrm(int sat, raw_t *raw)
{
    unsigned char *p=raw->buff+6,*q;
    int i,j,n,id=(U4(p+6)>>2)&0x7;
    
    //trace(4,"save_subfrm: sat=%2d id=%d\n",sat,id);
    
    if (id<1||5<id) return 0;
    
    q=raw->subfrm[sat-1]+(id-1)*30;
    
    for (i=n=0,p+=2;i<10;i++,p+=4) {
        for (j=23;j>=0;j--) {
            *q=(*q<<1)+((U4(p)>>j)&1); if (++n%8==0) q++;
        }
    }
    return id;
}
/* decode ephemeris ----------------------------------------------------------*/
static int decode_ephem(int sat, raw_t *raw)
{
    eph_t eph={0};
    
    //trace(4,"decode_ephem: sat=%2d\n",sat);
    
    if (decode_frame(raw->subfrm[sat-1]   ,&eph,NULL,NULL,NULL,NULL)!=1||
        decode_frame(raw->subfrm[sat-1]+30,&eph,NULL,NULL,NULL,NULL)!=2||
        decode_frame(raw->subfrm[sat-1]+60,&eph,NULL,NULL,NULL,NULL)!=3) return 0;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}

static int DecodeGeoEph(raw_t *raw,eph_t *ptEph)
{
	
	return 0;
}

static int DecodeMeoEph(raw_t *raw,eph_t *ptEph)
{
	
	return 0;
}
/* decode ephemeris ----------------------------------------------------------*/
static int decode_BDephem(int sat, raw_t *raw)
{
    eph_t eph={0};
    
    //trace(4,"decode_ephem: sat=%2d\n",sat);

	// BD GEOD?¨°???
	if (sat>=159 && sat< 163)
	{
		if(DecodeGeoEph(raw,&eph)!=1) return 0;
	}
	// BD ¡¤?GEOD?¨°???
	else if (sat>=33 && sat<=64)
	{
		if(DecodeMeoEph(raw,&eph)!=1) return 0;
	}

    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}
/* decode almanac and ion/utc ------------------------------------------------*/
static int decode_alm1(int sat, raw_t *raw)
{
    #if 1
	//trace(4,"decode_alm1 : sat=%2d\n",sat);
    decode_frame(raw->subfrm[sat-1]+90,NULL,raw->nav.alm,raw->nav.ion_gps,
                 raw->nav.utc_gps,&raw->nav.leaps);
	#endif

    return 0;
}
/* decode almanac ------------------------------------------------------------*/
static int decode_alm2(int sat, raw_t *raw)
{
    #if 1
	//trace(4,"decode_alm2 : sat=%2d\n",sat);
    decode_frame(raw->subfrm[sat-1]+120,NULL,raw->nav.alm,NULL,NULL,NULL);
	#endif
    return  0;
}
/* decode ublox rxm-sfrb: subframe buffer ------------------------------------*/
static int decode_rxmsfrb(raw_t *raw)
{
    unsigned int words[10];
    int i,prn,sat,sys,id;
    unsigned char *p=raw->buff+6;
    
    //trace(4,"decode_rxmsfrb: len=%d\n",raw->len);
    
    if (raw->len<42) {
        //trace(2,"ubx rxmsfrb length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U1(p+1);
    if (!(sat=satno(MINPRNSBS<=prn?SYS_SBS:SYS_GPS,prn))) {
        //trace(2,"ubx rxmsfrb satellite number error: prn=%d\n",prn);
        return -1;
    }
    sys=satsys(sat,&prn);
    
    if (sys==SYS_GPS) {
        id=save_subfrm(sat,raw);
        if (id==3) return decode_ephem(sat,raw);
        if (id==4) return decode_alm1 (sat,raw);
        if (id==5) return decode_alm2 (sat,raw);
        return 0;
    }
    else if (sys==SYS_SBS) {
        for (i=0,p+=2;i<10;i++,p+=4) words[i]=U4(p);
        return 0;
    }
	else if (sys==SYS_CMP)
	{
		id=save_subfrm(sat,raw);
        if (id==3) return decode_BDephem(sat,raw);
	}
    return 0;
}

/* decode gps and qzss navigation data ---------------------------------------*/
static int decode_nav(raw_t *raw, int sat, int off)
{
    unsigned int words[10];
    int i,id;
    unsigned char *p=raw->buff+6+off;
    
    if (raw->len<48+off) {
        //trace(2,"ubx rawsfrbx length error: sat=%d len=%d\n",sat,raw->len);
        return -1;
    }
    for (i=0;i<10;i++,p+=4) words[i]=U4(p)>>6; /* 24 bits without parity */
    
    id=(words[1]>>2)&7;
    if (id<1||5<id) {
        //trace(2,"ubx rawsfrbx subfrm id error: sat=%2d\n",sat);
        return -1;
    }
    for (i=0;i<10;i++) {
        setbitu(raw->subfrm[sat-1]+(id-1)*30,i*24,24,words[i]);
    }
    if (id==3) return decode_ephem(sat,raw);
    if (id==4) return decode_alm1 (sat,raw);
    if (id==5) return decode_alm2 (sat,raw);
    return 0;
}
/* decode galileo navigation data --------------------------------------------*/
static int decode_enav(raw_t *raw, int sat, int off)
{
    //trace(2,"ubx rawsfrbx galileo nav not supported sat=%d\n",sat);
    return 0;
}
/* decode beidou navigation data ---------------------------------------------*/
static int decode_cnav(raw_t *raw, int sat, int off)
{
    eph_t eph={0};
    unsigned int words[10];
    int i,id,pgn,prn;
    unsigned char *p=raw->buff+6+off;
    
    if (raw->len<48+off) {
        //trace(2,"ubx rawsfrbx length error: sat=%d len=%d\n",sat,raw->len);
        return -1;
    }
    for (i=0;i<10;i++,p+=4) words[i]=U4(p)&0x3FFFFFFF; /* 30 bits */
    
    satsys(sat,&prn);
    id=(words[0]>>12)&0x07; /* subframe id (3bit) */
    if (id<1||5<id) {
        //trace(2,"ubx rawsfrbx subfrm id error: sat=%2d\n",sat);
        return -1;
    }
    if (prn>=5) { /* IGSO/MEO */
        
        for (i=0;i<10;i++) {
            setbitu(raw->subfrm[sat-1]+(id-1)*38,i*30,30,words[i]);
        }
        if (id!=3) return 0;
        
        /* decode beidou D1 ephemeris */
        if (!decode_bds_d1(raw->subfrm[sat-1],&eph)) return 0;
    }
    else { /* GEO */
        if (id!=1) return 0;
        
        /* subframe 1 */
        pgn=(words[1]>>14)&0x0F; /* page number (4bit) */
        if (pgn<1||10<pgn) {
            //trace(2,"ubx rawsfrbx page number error: sat=%2d\n",sat);
            return -1;
        }
        for (i=0;i<10;i++) {
            setbitu(raw->subfrm[sat-1]+(pgn-1)*38,i*30,30,words[i]);
        }
        if (pgn!=10) return 0;
        
        /* decode beidou D2 ephemeris */
        if (!decode_bds_d2(raw->subfrm[sat-1],&eph)) return 0;
    }
    if (!strstr(raw->opt,"-EPHALL")) {
        if (timediff(eph.toe,raw->nav.eph[sat-1].toe)==0.0) return 0; /* unchanged */
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}
/* decode glonass navigation data --------------------------------------------*/
static int decode_gnav(raw_t *raw, int sat, int off, int frq)
{
#if 0
	geph_t geph={0};
    int i,j,k,m,prn;
    unsigned char *p=raw->buff+6+off,buff[64],*fid;
    
    satsys(sat,&prn);
    
    if (raw->len<24+off) {
        //trace(2,"ubx rawsfrbx gnav length error: len=%d\n",raw->len);
        return -1;
    }
    for (i=k=0;i<4;i++,p+=4) for (j=0;j<4;j++) {
        buff[k++]=p[3-j];
    }
    /* test hamming of glonass string */
    if (!test_glostr(buff)) {
        //trace(2,"ubx rawsfrbx glo string hamming error: sat=%2d\n",sat);
        return -1;
    }
    m=getbitu(buff,1,4);
    if (m<1||15<m) {
        trace(2,"ubx rawsfrbx glo string no error: sat=%2d\n",sat);
        return -1;
    }
    /* flush frame buffer if frame-id changed */
    fid=raw->subfrm[sat-1]+150;
    if (fid[0]!=buff[12]||fid[1]!=buff[13]) {
        for (i=0;i<4;i++) memset(raw->subfrm[sat-1]+i*10,0,10);
        memcpy1(fid,buff+12,2); /* save frame-id */
    }
    memcpy1(raw->subfrm[sat-1]+(m-1)*10,buff,10);
    
    if (m!=4) return 0;
    
    /* decode glonass ephemeris strings */
    geph.tof=raw->time;
    if (!decode_glostr(raw->subfrm[sat-1],&geph)||geph.sat!=sat) return 0;
    geph.frq=frq-7;
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (geph.iode==raw->nav.geph[prn-1].iode) return 0; /* unchanged */
    }
    raw->nav.geph[prn-1]=geph;
    raw->ephsat=sat;
#endif
    return 2;
}
/* decode sbas navigation data -----------------------------------------------*/
static int decode_snav(raw_t *raw, int sat, int off)
{
    #if 0
	int i,j,k,prn,tow,week;
    unsigned char *p=raw->buff+6+off,buff[64];
    
    if (raw->len<40+off) {
        //trace(2,"ubx rawsfrbx snav length error: len=%d\n",raw->len);
        return -1;
    }
    tow=(int)time2gpst(timeadd(raw->time,-1.0),&week);
    satsys(sat,&prn);
    raw->sbsmsg.prn=prn;
    raw->sbsmsg.tow=tow;
    raw->sbsmsg.week=week;
    for (i=k=0;i<8;i++,p+=4) for (j=0;j<4;j++) {
        buff[k++]=p[3-j];
    }
    memcpy1(raw->sbsmsg.msg,buff,29);
    raw->sbsmsg.msg[28]&=0xC0;
		#endif
    return 3;
}
/* decode ubx-rxm-sfrbx: raw subframe data (ref [3]) -------------------------*/
static int decode_rxmsfrbx(raw_t *raw)
{
    int prn,sat,sys,flag;
    unsigned char *p=raw->buff+6;
    
    //trace(4,"decode_rxmsfrbx: len=%d\n",raw->len);
    
    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX RXM-SFRBX (%4d): sys=%d prn=%3d",raw->len,
			U1(p),U1(p+1));
    }
    if (!(sys=ubx_sys(U1(p)))) {
        //trace(2,"ubx rxmsfrbx sys id error: sys=%d\n",U1(p));
        return -1;
    }
    prn=U1(p+1)+(sys==SYS_QZS?192:0);
    if (!(sat=satno(sys,prn))) {
        //trace(2,"ubx rxmsfrbx sat number error: sys=%d prn=%d\n",sys,prn);
        return -1;
    }
    switch (sys) {
		case SYS_GPS: 
			 flag = decode_nav (raw,sat,8);
			 if (flag == 2)
			 {
				 memcpy1(&nav.eph,&raw->nav.eph,sizeof(nav.eph));
				 nav.n=MAXSAT*1;
				 nav.nmax=0;
				 nav.ng=NSATGLO*1;
				 nav.ngmax=0;
				 nav.ns=NSATSBS*1;
				 nav.nsmax=0;
			 }
			 return flag;
		//case SYS_QZS: return decode_nav (raw,sat,8);
		//case SYS_GAL: return decode_enav(raw,sat,8);
		case SYS_CMP: 			
			 flag =decode_cnav(raw,sat,8);
			 if (flag == 2)
			 {
				 memcpy1(&nav.eph,&raw->nav.eph,sizeof(nav.eph));
				 nav.n=MAXSAT*1;
				 nav.nmax=0;
				 nav.ng=NSATGLO*1;
				 nav.ngmax=0;
				 nav.ns=NSATSBS*1;
				 nav.nsmax=0;
			 }
			 
			 return flag;
		//case SYS_GLO: return decode_gnav(raw,sat,8,U1(p+3));
		//case SYS_SBS: return decode_snav(raw,sat,8);

		
    }
    return 0;
} 
/* decode ublox raw message --------------------------------------------------*/
extern int decode_ubx(raw_t *raw,unsigned short usFlag)
{
    int type=(U1(raw->buff+2)<<8)+U1(raw->buff+3);
    //trace(3,"decode_ubx: type=%04x len=%d\n",type,raw->len);
    /* checksum */
    // LogStr("ubx: type=%04x len=%d\n",type,raw->len);
    if (!checksum(raw->buff,raw->len)) {
        //trace(2,"ubx checksum error: type=%04x len=%d\n",type,raw->len);
        //LogStr("checksum ubx checksum error: type=%04x len=%d\n",type,raw->len);
        return -1;
    }
    if(g_tBasePose.uiStationType == 1)
    {
        switch (type) 
        {
		    //case ID_RXMRAW  : return decode_rxmraw  (raw);
		    case ID_RXMRAWX : return decode_rxmrawx (raw,usFlag);
		    //case ID_RXMSFRB : return decode_rxmsfrb (raw);
		    case ID_RXMSFRBX: return decode_rxmsfrbx(raw);
		    case ID_POSBASE : return decode_base_pos(raw);
            case ID_VIEW_RTK :  return decode_view_rtk(raw);				
		    //case ID_POSECEF : return decode_rxposecef(raw);
            //case ID_NAVSOL  : return decode_navsol  (raw);
            //case ID_NAVTIME : return decode_navtime (raw);
            //case ID_TRKMEAS : return decode_trkmeas (raw);
            //case ID_TRKD5   : return decode_trkd5   (raw);
            //case ID_TRKSFRBX: return decode_trksfrbx(raw);
        }    		
    }
    else
    {
         switch (type) 
            {				
				case ID_POSECEF : return decode_rxposecef(raw);
                case ID_VIEW_RTK: return decode_view_rtk(raw);
            }         
    }

    if (raw->outtype) {
        sprintf(raw->msgtype,"UBX 0x%02X 0x%02X (%4d)",type>>8,type&0xF,
			raw->len);
    }
    return 0;
}
/* sync code -----------------------------------------------------------------*/
static int sync_ubx(unsigned char *buff, unsigned char data)
{
    buff[0]=buff[1]; buff[1]=data;
    return buff[0]==UBXSYNC1&&buff[1]==UBXSYNC2;
}
/* input ublox raw message from stream -----------------------------------------
* fetch next ublox raw data and input a mesasge from stream
* args   : raw_t *raw   IO     receiver raw data control struct
*          unsigned char data I stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*
* notes  : to specify input options, set raw->opt to the following option
*          strings separated by spaces.
*
*          -EPHALL    : input all ephemerides
*          -INVCP     : invert polarity of carrier-phase
*          -TADJ=tint : adjust time tags to multiples of tint (sec)
*
*-----------------------------------------------------------------------------*/
extern int input_ubx(raw_t *raw, unsigned char data,unsigned short usFlag)
{
    //trace(5,"input_ubx: data=%02x\n",data);
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        if (!sync_ubx(raw->buff,data)) return 0;
        raw->nbyte=2;

        return 0;
    }
    raw->buff[raw->nbyte++]=data; 

    if (raw->nbyte==6) {
        if ((raw->len=U2(raw->buff+4)+8)>MAXRAWLEN) {
            //trace(2,"ubx length error: len=%d\n",raw->len);
            //LogStr("ubx length error: len=%d\n",raw->len);
            raw->nbyte=0;
            return -1;
        }
    }
	// LogStr("ubx length len=%d\n",raw->len);
    if (raw->nbyte<6||raw->nbyte<raw->len) return 0;
    raw->nbyte=0;
  
    /* decode ublox raw message */
    
    /* decode ublox raw message */
    return decode_ubx(raw,usFlag);
}
/* input ublox raw message from file -------------------------------------------
* fetch next ublox raw data and input a message from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          FILE   *fp    I      file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_ubxf(raw_t *raw, FILE *fp,unsigned short usFlag)
{
    int i,data,temp;
    
    //trace(4,"input_ubxf:\n");
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_ubx(raw->buff,(unsigned char)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (fread(raw->buff+2,1,4,fp)<4) return -2;
    raw->nbyte=6;
    
    if ((raw->len=U2(raw->buff+4)+8)>MAXRAWLEN) {
        //trace(2,"ubx length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (fread(raw->buff+6,1,raw->len-6,fp)<(size_t)(raw->len-6)) return -2;
    raw->nbyte=0;
    
    /* decode ubx raw message */
    return decode_ubx(raw,usFlag);
}
/* generate ublox binary message -----------------------------------------------
* generate ublox binary message from message string
* args   : char  *msg   IO     message string 
*            "CFG-PRT   portid res0 res1 mode baudrate inmask outmask flags"
*            "CFG-USB   vendid prodid res1 res2 power flags vstr pstr serino"
*            "CFG-MSG   msgid rate0 rate1 rate2 rate3"
*            "CFG-NMEA  filter version numsv flags"
*            "CFG-RATE  meas nav time"
*            "CFG-CFG   clear_mask save_mask load_mask"
*            "CFG-TP    interval length status time_ref res adelay rdelay udelay"
*            "CFG-NAV2  ..."
*            "CFG-DAT   maja flat dx dy dz rotx roty rotz scale"
*            "CFG-INF   protocolid res0 res1 mask0 mask1 mask2 mask3"
*            "CFG-RST   navbbr reset res"
*            "CFG-RXM   gpsmode lpmode"
*            "CFG-ANT   flags pins"
*            "CFG-FXN   flags treacq tacq treacqoff tacqoff ton toff res basetow"
*            "CFG-SBAS  mode usage maxsbas res scanmode"
*            "CFG-LIC   key0 key1 key2 key3 key4 key5"
*            "CFG-TM    intid rate flags"
*            "CFG-TM2   ch res0 res1 rate flags"
*            "CFG-TMODE tmode posx posy posz posvar svinmindur svinvarlimit"
*            "CFG-EKF   ..."
*          unsigned char *buff O binary message
* return : length of binary message (0: error)
* note   : see reference [1] for details.
*-----------------------------------------------------------------------------*/
extern int gen_ubx(const char *msg, unsigned char *buff)
{
#if 0
	const char *cmd[]={
        "PRT","USB","MSG","NMEA","RATE","CFG","TP","NAV2","DAT","INF",
        "RST","RXM","ANT","FXN","SBAS","LIC","TM","TM2","TMODE","EKF",""
    };
    const unsigned char id[]={
        0x00,0x1B,0x01,0x17,0x08,0x09,0x07,0x1A,0x06,0x02,
        0x04,0x11,0x13,0x0E,0x16,0x80,0x10,0x19,0x1D,0x12
    };
    const int prm[][32]={
        {FU1,FU1,FU2,FU4,FU4,FU2,FU2,FU2,FU2},    /* PRT */
        {FU2,FU2,FU2,FU2,FU2,FU2,FS32,FS32,FS32}, /* USB */
        {FU1,FU1,FU1,FU1,FU1,FU1},                /* MSG */
        {FU1,FU1,FU1,FU1},                        /* NMEA */
        {FU2,FU2,FU2},                            /* RATE */
        {FU4,FU4,FU4},                            /* CFG */
        {FU4,FU4,FI1,FU1,FU2,FI2,FI2,FI4},        /* TP */
        {FU1,FU1,FU2,FU1,FU1,FU1,FU1,FI4,FU1,FU1,FU1,FU1,FU1,FU1,FU2,FU2,FU2,FU2,
         FU2,FU1,FU1,FU2,FU4,FU4},                /* NAV2 */
        {FR8,FR8,FR4,FR4,FR4,FR4,FR4,FR4,FR4},    /* DAT */
        {FU1,FU1,FU2,FU1,FU1,FU1,FU1},            /* INF */
        {FU2,FU1,FU1},                            /* RST */
        {FU1,FU1},                                /* RXM */
        {FU2,FU2},                                /* ANT */
        {FU4,FU4,FU4,FU4,FU4,FU4,FU4,FU4},        /* FXN */
        {FU1,FU1,FU1,FU1,FU4},                    /* SBAS */
        {FU2,FU2,FU2,FU2,FU2,FU2},                /* LIC */
        {FU4,FU4,FU4},                            /* TM */
        {FU1,FU1,FU2,FU4,FU4},                    /* TM2 */
        {FU4,FI4,FI4,FI4,FU4,FU4,FU4},            /* TMODE */
        {FU1,FU1,FU1,FU1,FU4,FU2,FU2,FU1,FU1,FU2} /* EKF */
    };
    unsigned char *q=buff;
    char mbuff[1024],*args[32],*p;
    int i,j,n,narg=0;
    
    //trace(4,"gen_ubxf: msg=%s\n",msg);
    
    strcpy(mbuff,msg);
    for (p=strtok(mbuff," ");p&&narg<32;p=strtok(NULL," ")) {
        args[narg++]=p;
    }
    if (narg<1||strncmp(args[0],"CFG-",4)) return 0;
    
    for (i=0;*cmd[i];i++) {
        if (!strcmp(args[0]+4,cmd[i])) break;
    }
    if (!*cmd[i]) return 0;
    
    *q++=UBXSYNC1;
    *q++=UBXSYNC2;
    *q++=UBXCFG;
    *q++=id[i];
    q+=2;
    for (j=1;prm[i][j-1]>0;j++) {
        switch (prm[i][j-1]) {
            case FU1 : setU1(q,j<narg?(unsigned char )atoi(args[j]):0); q+=1; break;
            case FU2 : setU2(q,j<narg?(unsigned short)atoi(args[j]):0); q+=2; break;
            case FU4 : setU4(q,j<narg?(unsigned int  )atoi(args[j]):0); q+=4; break;
            case FI1 : setI1(q,j<narg?(char          )atoi(args[j]):0); q+=1; break;
            case FI2 : setI2(q,j<narg?(short         )atoi(args[j]):0); q+=2; break;
            case FI4 : setI4(q,j<narg?(int           )atoi(args[j]):0); q+=4; break;
            case FR4 : setR4(q,j<narg?(float         )atof(args[j]):0); q+=4; break;
            case FR8 : setR8(q,j<narg?(double)atof(args[j]):0); q+=8; break;
            case FS32: sprintf((char *)q,"%-32.32s",j<narg?args[j]:""); q+=32; break;
        }
    }
    n=(int)(q-buff)+2;
    setU2(buff+4,(unsigned short)(n-8));
    setcs(buff,n);
    
    //trace(5,"gen_ubxf: buff=\n"); traceb(5,buff,n);
    return n;
#endif
	return 1;
}

/* decode navigation data subframe 1 -----------------------------------------*/
static int decode_subfrm1(const unsigned char *buff, eph_t *eph)
{
    double tow,toc;
    int i=48,week,iodc0,iodc1;
    
    //trace(4,"decode_subfrm1:\n");
    //trace(5,"decode_subfrm1: buff="); traceb(5,buff,30);
    
    tow        =getbitu(buff,24,17)*6.0;           /* transmission time */
    week       =getbitu(buff,i,10);       i+=10;
    eph->code  =getbitu(buff,i, 2);       i+= 2;
    eph->sva   =getbitu(buff,i, 4);       i+= 4;   /* ura index */
    eph->svh   =getbitu(buff,i, 6);       i+= 6;
    iodc0      =getbitu(buff,i, 2);       i+= 2;
    eph->flag  =getbitu(buff,i, 1);       i+= 1+87;
    eph->tgd[0]=getbits(buff,i, 8)*P2_31; i+= 8;
    iodc1      =getbitu(buff,i, 8);       i+= 8;
    toc        =getbitu(buff,i,16)*16.0;  i+=16;
    eph->f2    =getbits(buff,i, 8)*P2_55; i+= 8;
    eph->f1    =getbits(buff,i,16)*P2_43; i+=16;
    eph->f0    =getbits(buff,i,22)*P2_31;
    
    eph->iodc=(iodc0<<8)+iodc1;
    eph->week=week+1024; /* week of tow */
    eph->ttr=gpst2time(eph->week,tow);
    eph->toc=gpst2time(eph->week,toc);
    
    return 1;
}
/* decode navigation data subframe 2 -----------------------------------------*/
static int decode_subfrm2(const unsigned char *buff, eph_t *eph)
{
    double sqrtA;
    int i=48;
    
    //trace(4,"decode_subfrm2:\n");
    //trace(5,"decode_subfrm2: buff="); traceb(5,buff,30);
    
    eph->iode=getbitu(buff,i, 8);              i+= 8;
    eph->crs =getbits(buff,i,16)*P2_5;         i+=16;
    eph->deln=getbits(buff,i,16)*P2_43*SC2RAD; i+=16;
    eph->M0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph->cuc =getbits(buff,i,16)*P2_29;        i+=16;
    eph->e   =getbitu(buff,i,32)*P2_33;        i+=32;
    eph->cus =getbits(buff,i,16)*P2_29;        i+=16;
    sqrtA    =getbitu(buff,i,32)*P2_19;        i+=32;
    eph->toes=getbitu(buff,i,16)*16.0;         i+=16;
    eph->fit =getbitu(buff,i, 1)?0.0:4.0; /* 0:4hr,1:>4hr */
    
    eph->A=sqrtA*sqrtA;
    
    return 2;
}
/* decode navigation data subframe 3 -----------------------------------------*/
static int decode_subfrm3(const unsigned char *buff, eph_t *eph)
{
    double tow,toc;
    int i=48,iode;
    
    //trace(4,"decode_subfrm3:\n");
    //trace(5,"decode_subfrm3: buff="); traceb(5,buff,30);
    
    eph->cic =getbits(buff,i,16)*P2_29;        i+=16;
    eph->OMG0=getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph->cis =getbits(buff,i,16)*P2_29;        i+=16;
    eph->i0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph->crc =getbits(buff,i,16)*P2_5;         i+=16;
    eph->omg =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    eph->OMGd=getbits(buff,i,24)*P2_43*SC2RAD; i+=24;
    iode     =getbitu(buff,i, 8);              i+= 8;
    eph->idot=getbits(buff,i,14)*P2_43*SC2RAD;
    
    /* check iode and iodc consistency */
    if (iode!=eph->iode||iode!=(eph->iodc&0xFF)) return 0;
    
    /* adjustment for week handover */
    tow=time2gpst(eph->ttr,&eph->week);
    toc=time2gpst(eph->toc,NULL);
    if      (eph->toes<tow-302400.0) {eph->week++; tow-=604800.0;}
    else if (eph->toes>tow+302400.0) {eph->week--; tow+=604800.0;}
    eph->toe=gpst2time(eph->week,eph->toes);
    eph->toc=gpst2time(eph->week,toc);
    eph->ttr=gpst2time(eph->week,tow);
    
    return 3;
}
/* decode almanac ------------------------------------------------------------*/
static void decode_almanac(const unsigned char *buff, alm_t *alm)
{
    gtime_t toa;
    double deltai,sqrtA,tt;
    int i=50,f0,sat=getbitu(buff,50,6);
    
    //trace(4,"decode_almanac: sat=%2d\n",sat);
    
    if (!alm||sat<1||32<sat||alm[sat-1].week==0) return;
    
    alm[sat-1].sat =sat;
    alm[sat-1].e   =getbits(buff,i,16)*P2_21;        i+=16;
    alm[sat-1].toas=getbitu(buff,i, 8)*4096.0;       i+= 8;
    deltai         =getbits(buff,i,16)*P2_19*SC2RAD; i+=16;
    alm[sat-1].OMGd=getbits(buff,i,16)*P2_38*SC2RAD; i+=16;
    alm[sat-1].svh =getbitu(buff,i, 8);              i+= 8;
    sqrtA          =getbitu(buff,i,24)*P2_11;        i+=24;
    alm[sat-1].OMG0=getbits(buff,i,24)*P2_23*SC2RAD; i+=24;
    alm[sat-1].omg =getbits(buff,i,24)*P2_23*SC2RAD; i+=24;
    alm[sat-1].M0  =getbits(buff,i,24)*P2_23*SC2RAD; i+=24;
    f0             =getbitu(buff,i, 8);              i+= 8;
    alm[sat-1].f1  =getbits(buff,i,11)*P2_38;        i+=11;
    alm[sat-1].f0  =getbits(buff,i, 3)*P2_17+f0*P2_20;
    alm[sat-1].A   =sqrtA*sqrtA;
    alm[sat-1].i0  =0.3*SC2RAD+deltai;
    
    toa=gpst2time(alm[sat-1].week,alm[sat-1].toas);
    tt=timediff(toa,alm[sat-1].toa);
    if      (tt<302400.0) alm[sat-1].week--;
    else if (tt>302400.0) alm[sat-1].week++;
    alm[sat-1].toa=gpst2time(alm[sat-1].week,alm[sat-1].toas);
}
/* decode navigation data subframe 4 -----------------------------------------*/
static int decode_subfrm4(const unsigned char *buff, alm_t *alm, double *ion,
                          double *utc, int *leaps)
{
    int i,sat,svid=getbitu(buff,50,6);
    
    //trace(4,"decode_subfrm4: svid=%d\n",svid);
    //trace(5,"decode_subfrm4: buff="); traceb(5,buff,30);
    
    if (25<=svid&&svid<=32) { /* page 2,3,4,5,7,8,9,10 */
        
        /* decode almanac */
        //decode_almanac(buff,alm);
    }
    else if (svid==63) { /* page 25 */
        
        /* decode as and sv config */
        i=56;
        for (sat=1;sat<=32;sat++) {
            if (alm) alm[sat-1].svconf=getbitu(buff,i,4); i+=4;
        }
        /* decode sv health */
        i=186;
        for (sat=25;sat<=32;sat++) {
            if (alm) alm[sat-1].svh   =getbitu(buff,i,6); i+=6;
        }
    }
    else if (svid==56) { /* page 18 */
        
        /* decode ion/utc parameters */
        if (ion) {
            i=56;
            ion[0]=getbits(buff,i, 8)*P2_30;     i+= 8;
            ion[1]=getbits(buff,i, 8)*P2_27;     i+= 8;
            ion[2]=getbits(buff,i, 8)*P2_24;     i+= 8;
            ion[3]=getbits(buff,i, 8)*P2_24;     i+= 8;
            ion[4]=getbits(buff,i, 8)*pow(2,11); i+= 8;
            ion[5]=getbits(buff,i, 8)*pow(2,14); i+= 8;
            ion[6]=getbits(buff,i, 8)*pow(2,16); i+= 8;
            ion[7]=getbits(buff,i, 8)*pow(2,16);
        }
        if (utc) {
            i=120;
            utc[1]=getbits(buff,i,24)*P2_50;     i+=24;
            utc[0]=getbits(buff,i,32)*P2_30;     i+=32;
            utc[2]=getbits(buff,i, 8)*pow(2,12); i+= 8;
            utc[3]=getbitu(buff,i, 8);
        }
        if (leaps) {
            i=192;
            *leaps=getbits(buff,i,8);
        }
    }
    return 4;
}
/* decode navigation data subframe 5 -----------------------------------------*/
static int decode_subfrm5(const unsigned char *buff, alm_t *alm)
{
    double toas;
    int i,sat,week,svid=getbitu(buff,50,6);
    
    //trace(4,"decode_subfrm5: svid=%d\n",svid);
    //trace(5,"decode_subfrm5: buff="); traceb(5,buff,30);
    
    if (1<=svid&&svid<=24) { /* page 1-24 */
        
        /* decode almanac */
        decode_almanac(buff,alm);
    }
    else if (svid==51) { /* page 25 */
        
        if (alm) {
            i=56;
            toas=getbitu(buff,i,8)*4096; i+=8;
            week=getbitu(buff,i,8);      i+=8;
            //week=week;
            
            /* decode sv health */
            for (sat=1;sat<=24;sat++) {
                alm[sat-1].svh=getbitu(buff,i,6); i+=6;
            }
            for (sat=1;sat<=32;sat++) {
                alm[sat-1].toas=toas;
                alm[sat-1].week=week;
                alm[sat-1].toa=gpst2time(week,toas);
            }
        }
    }
    return 5;
}  

/* decode navigation data frame ------------------------------------------------
* decode navigation data frame and extract ephemeris and ion/utc parameters
* args   : unsigned char *buff I navigation data frame without parity (8bitx30)
*          eph_t *eph    IO     ephemeris message      (NULL: no input)
*          alm_t *alm    IO     almanac                (NULL: no input)
*          double *ion   IO     ionospheric parameters (NULL: no input)
*          double *utc   IO     delta-utc parameters   (NULL: no input)
*          int   *leaps  IO     leap seconds (s)       (NULL: no input)
* return : status (0:no valid, 1-5:subframe id)
* notes  : use cpu time to resolve modulo 1024 ambiguity of the week number
*          see ref [1]
*          utc[3] reference week for utc parameter is truncated in 8 bits
*-----------------------------------------------------------------------------*/
extern int decode_frame(const unsigned char *buff, eph_t *eph, alm_t *alm,
                        double *ion, double *utc, int *leaps)
{
    int id=getbitu(buff,43,3); /* subframe id */
    
    //trace(3,"decodefrm: id=%d\n",id);
    
    switch (id) {
	case 1: return decode_subfrm1(buff,eph);
	case 2: return decode_subfrm2(buff,eph);
	case 3: return decode_subfrm3(buff,eph);
	case 4: return decode_subfrm4(buff,alm,ion,utc,leaps);
	case 5: return decode_subfrm5(buff,alm);
    }
    return 0;
}


/* decode BeiDou D1 ephemeris --------------------------------------------------
* decode BeiDou D1 ephemeris (IGSO/MEO satellites) (ref [3] 5.2)
* args   : unsigned char *buff I beidou D1 subframe bits
*                                  buff[ 0- 37]: subframe 1 (300 bits)
*                                  buff[38- 75]: subframe 2
*                                  buff[76-113]: subframe 3
*          eph_t    *eph    IO  ephemeris structure
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int decode_bds_d1(const unsigned char *buff, eph_t *eph)
{
    double toc_bds,sqrtA;
    unsigned int toe1,toe2,sow1,sow2,sow3;
    int i,frn1,frn2,frn3;
    
    //trace(3,"decode_bds_d1:\n");
    
    i=8*38*0; /* subframe 1 */
    frn1       =getbitu (buff,i+ 15, 3);
    sow1       =getbitu2(buff,i+ 18, 8,i+30,12);
    eph->svh   =getbitu (buff,i+ 42, 1); /* SatH1 */
    eph->iodc  =getbitu (buff,i+ 43, 5); /* AODC */
    eph->sva   =getbitu (buff,i+ 48, 4);
    eph->week  =getbitu (buff,i+ 60,13); /* week in BDT */
    toc_bds    =getbitu2(buff,i+ 73, 9,i+ 90, 8)*8.0;
    eph->tgd[0]=getbits (buff,i+ 98,10)*0.1*1E-9;
    eph->tgd[1]=getbits2(buff,i+108, 4,i+120, 6)*0.1*1E-9;
    eph->f2    =getbits (buff,i+214,11)*P2_66;
    eph->f0    =getbits2(buff,i+225, 7,i+240,17)*P2_33;
    eph->f1    =getbits2(buff,i+257, 5,i+270,17)*P2_50;
    eph->iode  =getbitu (buff,i+287, 5); /* AODE */
    
    i=8*38*1; /* subframe 2 */
    frn2       =getbitu (buff,i+ 15, 3);
    sow2       =getbitu2(buff,i+ 18, 8,i+30,12);
    eph->deln  =getbits2(buff,i+ 42,10,i+ 60, 6)*P2_43*SC2RAD;
    eph->cuc   =getbits2(buff,i+ 66,16,i+ 90, 2)*P2_31;
    eph->M0    =getbits2(buff,i+ 92,20,i+120,12)*P2_31*SC2RAD;
    eph->e     =getbitu2(buff,i+132,10,i+150,22)*P2_33;
    eph->cus   =getbits (buff,i+180,18)*P2_31;
    eph->crc   =getbits2(buff,i+198, 4,i+210,14)*P2_6;
    eph->crs   =getbits2(buff,i+224, 8,i+240,10)*P2_6;
    sqrtA      =getbitu2(buff,i+250,12,i+270,20)*P2_19;
    toe1       =getbitu (buff,i+290, 2); /* TOE 2-MSB */
    eph->A     =sqrtA*sqrtA;
    
    i=8*38*2; /* subframe 3 */
    frn3       =getbitu (buff,i+ 15, 3);
    sow3       =getbitu2(buff,i+ 18, 8,i+30,12);
    toe2       =getbitu2(buff,i+ 42,10,i+ 60, 5); /* TOE 5-LSB */
    eph->i0    =getbits2(buff,i+ 65,17,i+ 90,15)*P2_31*SC2RAD;
    eph->cic   =getbits2(buff,i+105, 7,i+120,11)*P2_31;
    eph->OMGd  =getbits2(buff,i+131,11,i+150,13)*P2_43*SC2RAD;
    eph->cis   =getbits2(buff,i+163, 9,i+180, 9)*P2_31;
    eph->idot  =getbits2(buff,i+189,13,i+210, 1)*P2_43*SC2RAD;
    eph->OMG0  =getbits2(buff,i+211,21,i+240,11)*P2_31*SC2RAD;
    eph->omg   =getbits2(buff,i+251,11,i+270,21)*P2_31*SC2RAD;
    eph->toes  =merge_two_u(toe1,toe2,15)*8.0;
    
    /* check consistency of subframe numbers, sows and toe/toc */
    if (frn1!=1||frn2!=2||frn3!=3) {
        //trace(3,"decode_bds_d1 error: frn=%d %d %d\n",frn1,frn2,frn3);
        return 0;
    }
    if (sow2!=sow1+6||sow3!=sow2+6) {
        //trace(3,"decode_bds_d1 error: sow=%d %d %d\n",sow1,sow2,sow3);
        return 0;
    }
    if (toc_bds!=eph->toes) {
        //trace(3,"decode_bds_d1 error: toe=%.0f toc=%.0f\n",eph->toes,toc_bds);
        return 0;
    }
    eph->ttr=bdt2gpst(bdt2time(eph->week,sow1));      /* bdt -> gpst */
    if      (eph->toes>sow1+302400.0) eph->week++;
    else if (eph->toes<sow1-302400.0) eph->week--;
    eph->toe=bdt2gpst(bdt2time(eph->week,eph->toes)); /* bdt -> gpst */
    eph->toc=bdt2gpst(bdt2time(eph->week,toc_bds));   /* bdt -> gpst */
    return 1;
}
/* decode BeiDou D2 ephemeris --------------------------------------------------
* decode BeiDou D2 ephemeris (GEO satellites) (ref [3] 5.3)
* args   : unsigned char *buff I beidou D2 subframe 1 page bits
*                                  buff[  0- 37]: page 1 (300 bits)
*                                  buff[ 38- 75]: page 2
*                                  ...
*                                  buff[342-379]: page 10
*          eph_t    *eph    IO  ephemeris structure
* return : status (1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int decode_bds_d2(const unsigned char *buff, eph_t *eph)
{
    double toc_bds,sqrtA;
    unsigned int f1p4,cucp5,ep6,cicp7,i0p8,OMGdp9,omgp10;
    unsigned int sow1,sow3,sow4,sow5,sow6,sow7,sow8,sow9,sow10;
    int i,f1p3,cucp4,ep5,cicp6,i0p7,OMGdp8,omgp9;
    int pgn1,pgn3,pgn4,pgn5,pgn6,pgn7,pgn8,pgn9,pgn10;
    
    //trace(3,"decode_bds_d2:\n");
    
    i=8*38*0; /* page 1 */
    pgn1       =getbitu (buff,i+ 42, 4);
    sow1       =getbitu2(buff,i+ 18, 8,i+ 30,12);
    eph->svh   =getbitu (buff,i+ 46, 1); /* SatH1 */
    eph->iodc  =getbitu (buff,i+ 47, 5); /* AODC */
    eph->sva   =getbitu (buff,i+ 60, 4);
    eph->week  =getbitu (buff,i+ 64,13); /* week in BDT */
    toc_bds    =getbitu2(buff,i+ 77, 5,i+ 90,12)*8.0;
    eph->tgd[0]=getbits (buff,i+102,10)*0.1*1E-9;
    eph->tgd[1]=getbits (buff,i+120,10)*0.1*1E-9;
    
    i=8*38*2; /* page 3 */
    pgn3       =getbitu (buff,i+ 42, 4);
    sow3       =getbitu2(buff,i+ 18, 8,i+ 30,12);
    eph->f0    =getbits2(buff,i+100,12,i+120,12)*P2_33;
    f1p3       =getbits (buff,i+132,4);
    
    i=8*38*3; /* page 4 */
    pgn4       =getbitu (buff,i+ 42, 4);
    sow4       =getbitu2(buff,i+ 18, 8,i+ 30,12);
    f1p4       =getbitu2(buff,i+ 46, 6,i+ 60,12);
    eph->f2    =getbits2(buff,i+ 72,10,i+ 90, 1)*P2_66;
    eph->iode  =getbitu (buff,i+ 91, 5); /* AODE */
    eph->deln  =getbits (buff,i+ 96,16)*P2_43*SC2RAD;
    cucp4      =getbits (buff,i+120,14);
    
    i=8*38*4; /* page 5 */
    pgn5       =getbitu (buff,i+ 42, 4);
    sow5       =getbitu2(buff,i+ 18, 8,i+ 30,12);
    cucp5      =getbitu (buff,i+ 46, 4);
    eph->M0    =getbits3(buff,i+ 50, 2,i+ 60,22,i+ 90, 8)*P2_31*SC2RAD;
    eph->cus   =getbits2(buff,i+ 98,14,i+120, 4)*P2_31;
    ep5        =getbits (buff,i+124,10);
    
    i=8*38*5; /* page 6 */
    pgn6       =getbitu (buff,i+ 42, 4);
    sow6       =getbitu2(buff,i+ 18, 8,i+ 30,12);
    ep6        =getbitu2(buff,i+ 46, 6,i+ 60,16);
    sqrtA      =getbitu3(buff,i+ 76, 6,i+ 90,22,i+120,4)*P2_19;
    cicp6      =getbits (buff,i+124,10);
    eph->A     =sqrtA*sqrtA;
    
    i=8*38*6; /* page 7 */
    pgn7       =getbitu (buff,i+ 42, 4);
    sow7       =getbitu2(buff,i+ 18, 8,i+ 30,12);
    cicp7      =getbitu2(buff,i+ 46, 6,i+ 60, 2);
    eph->cis   =getbits (buff,i+ 62,18)*P2_31;
    eph->toes  =getbitu2(buff,i+ 80, 2,i+ 90,15)*8.0;
    i0p7       =getbits2(buff,i+105, 7,i+120,14);
    
    i=8*38*7; /* page 8 */
    pgn8       =getbitu (buff,i+ 42, 4);
    sow8       =getbitu2(buff,i+ 18, 8,i+ 30,12);
    i0p8       =getbitu2(buff,i+ 46, 6,i+ 60, 5);
    eph->crc   =getbits2(buff,i+ 65,17,i+ 90, 1)*P2_6;
    eph->crs   =getbits (buff,i+ 91,18)*P2_6;
    OMGdp8     =getbits2(buff,i+109, 3,i+120,16);
    
    i=8*38*8; /* page 9 */
    pgn9       =getbitu (buff,i+ 42, 4);
    sow9       =getbitu2(buff,i+ 18, 8,i+ 30,12);
    OMGdp9     =getbitu (buff,i+ 46, 5);
    eph->OMG0  =getbits3(buff,i+ 51, 1,i+ 60,22,i+ 90, 9)*P2_31*SC2RAD;
    omgp9      =getbits2(buff,i+ 99,13,i+120,14);
    
    i=8*38*9; /* page 10 */
    pgn10      =getbitu (buff,i+ 42, 4);
    sow10      =getbitu2(buff,i+ 18, 8,i+ 30,12);
    omgp10     =getbitu (buff,i+ 46, 5);
    eph->idot  =getbits2(buff,i+ 51, 1,i+ 60,13)*P2_43*SC2RAD;
    
    /* check consistency of page numbers, sows and toe/toc */
    if (pgn1!=1||pgn3!=3||pgn4!=4||pgn5!=5||pgn6!=6||pgn7!=7||pgn8!=8||pgn9!=9||
        pgn10!=10) {
        //trace(3,"decode_bds_d2 error: pgn=%d %d %d %d %d %d %d %d %d\n",
        //      pgn1,pgn3,pgn4,pgn5,pgn6,pgn7,pgn8,pgn9,pgn10);
        return 0;
    }
    if (sow3!=sow1+6||sow4!=sow3+3||sow5!=sow4+3||sow6!=sow5+3||
        sow7!=sow6+3||sow8!=sow7+3||sow9!=sow8+3||sow10!=sow9+3) {
        //trace(3,"decode_bds_d2 error: sow=%d %d %d %d %d %d %d %d %d\n",
        //      sow1,sow3,sow4,sow5,sow6,sow7,sow8,sow9,sow10);
        return 0;
    }
    if (toc_bds!=eph->toes) {
        //trace(3,"decode_bds_d2 error: toe=%.0f toc=%.0f\n",eph->toes,toc_bds);
        return 0;
    }
    eph->f1  =merge_two_s(f1p3  ,f1p4  ,18)*P2_50;
    eph->cuc =merge_two_s(cucp4 ,cucp5 , 4)*P2_31;
    eph->e   =merge_two_s(ep5   ,ep6   ,22)*P2_33;
    eph->cic =merge_two_s(cicp6 ,cicp7 , 8)*P2_31;
    eph->i0  =merge_two_s(i0p7  ,i0p8  ,11)*P2_31*SC2RAD;
    eph->OMGd=merge_two_s(OMGdp8,OMGdp9, 5)*P2_43*SC2RAD;
    eph->omg =merge_two_s(omgp9 ,omgp10, 5)*P2_31*SC2RAD;
    
    eph->ttr=bdt2gpst(bdt2time(eph->week,sow1));      /* bdt -> gpst */
    if      (eph->toes>sow1+302400.0) eph->week++;
    else if (eph->toes<sow1-302400.0) eph->week--;
    eph->toe=bdt2gpst(bdt2time(eph->week,eph->toes)); /* bdt -> gpst */
    eph->toc=bdt2gpst(bdt2time(eph->week,toc_bds));   /* bdt -> gpst */
    return 1;
}


/* GT_free receiver raw data control ----------------------------------------------
* GT_free observation and ephemeris buffer in receiver raw data control struct
* args   : raw_t  *raw   IO     receiver raw data control struct
* return : none
*-----------------------------------------------------------------------------*/
extern void free_raw(raw_t *raw)
{
    //trace(3,"free_raw:\n");
    
    //GT_free(raw->obs.data ); raw->obs.data =NULL; raw->obs.n =0;
    //GT_free(raw->obuf.data); raw->obuf.data=NULL; raw->obuf.n=0;
    //GT_free(raw->nav.eph  ); raw->nav.eph  =NULL; raw->nav.n =0;
    //GT_free(raw->nav.alm  ); raw->nav.alm  =NULL; raw->nav.na=0;
    //GT_free(raw->nav.geph ); raw->nav.geph =NULL; raw->nav.ng=0;
    //GT_free(raw->nav.seph ); raw->nav.seph =NULL; raw->nav.ns=0;
}

/* initialize receiver raw data control ----------------------------------------
* initialize receiver raw data control struct and reallocate obsevation and
* epheris buffer
* args   : raw_t  *raw   IO     receiver raw data control struct
* return : status (1:ok,0:memory allocation error)
*-----------------------------------------------------------------------------*/
int init_raw(raw_t *raw)
{
#if 0
	const double lam_glo[3]={CLIGHT/FREQ1_GLO,CLIGHT/FREQ2_GLO,0};
    gtime_t time0={0};
    obsd_t data0={{0}};
    eph_t  eph0 ={0,-1,-1};
    alm_t  alm0 ={0,-1};
    geph_t geph0={0,-1};
    seph_t seph0={0};
    sbsmsg_t sbsmsg0={0};
    lexmsg_t lexmsg0={0};
    int i,j,sys;
    
    //trace(3,"init_raw:\n");
   
    raw->time=raw->tobs=time0;

    raw->ephsat=0;
    raw->sbsmsg=sbsmsg0;
    raw->msgtype[0]='\0';

    for (i=0;i<MAXSAT;i++) {
        for (j=0;j<150  ;j++) raw->subfrm[i][j]=0;
        for (j=0;j<NFREQ;j++) raw->lockt[i][j]=0.0;
        for (j=0;j<NFREQ;j++) raw->halfc[i][j]=0;
        raw->icpp[i]=raw->off[i]=raw->prCA[i]=raw->dpCA[i]=0.0;
    }

    for (i=0;i<MAXOBS;i++) raw->freqn[i]=0;
    raw->lexmsg=lexmsg0;
    raw->icpc=0.0;
    raw->nbyte=raw->len=0;
    raw->iod=raw->flag=raw->tbase=raw->outtype=0;
    raw->tod=-1;
    for (i=0;i<MAXRAWLEN;i++) raw->buff[i]=0;
    raw->opt[0]='\0';
    
    raw->obs.data =NULL;
    raw->obuf.data=NULL;
    raw->nav.eph  =NULL;
    raw->nav.alm  =NULL;
    raw->nav.geph =NULL;
    raw->nav.seph =NULL;
    
    if (!(raw->obs.data =(obsd_t *)GT_malloc(sizeof(obsd_t)*MAXOBS))||
        !(raw->obuf.data=(obsd_t *)GT_malloc(sizeof(obsd_t)*MAXOBS))||
        !(raw->nav.eph  =(eph_t  *)GT_malloc(sizeof(eph_t )*MAXSAT))||
        !(raw->nav.alm  =(alm_t  *)GT_malloc(sizeof(alm_t )*MAXSAT))||
        !(raw->nav.geph =(geph_t *)GT_malloc(sizeof(geph_t)*NSATGLO))||
        !(raw->nav.seph =(seph_t *)GT_malloc(sizeof(seph_t)*NSATSBS*2))) {
        free_raw(raw);
        return 0;
    }
//#if 1
    raw->obs.n =0;
    raw->obuf.n=0;
    raw->nav.n =MAXSAT;
    raw->nav.na=MAXSAT;
    raw->nav.ng=NSATGLO;
    raw->nav.ns=NSATSBS*2;
    for (i=0;i<MAXOBS   ;i++) raw->obs.data [i]=data0;
    for (i=0;i<MAXOBS   ;i++) raw->obuf.data[i]=data0;
    for (i=0;i<MAXSAT   ;i++) raw->nav.eph  [i]=eph0;
    for (i=0;i<MAXSAT   ;i++) raw->nav.alm  [i]=alm0;
    for (i=0;i<NSATGLO  ;i++) raw->nav.geph [i]=geph0;
    for (i=0;i<NSATSBS*2;i++) raw->nav.seph [i]=seph0;
    for (i=0;i<MAXSAT;i++) for (j=0;j<NFREQ;j++) {
        if (!(sys=satsys(i+1,NULL))) continue;
        raw->nav.lam[i][j]=sys==SYS_GLO?lam_glo[j]:lam_carr[j];
    }
    raw->sta.name[0]=raw->sta.marker[0]='\0';
    raw->sta.antdes[0]=raw->sta.antsno[0]='\0';
    raw->sta.rectype[0]=raw->sta.recver[0]=raw->sta.recsno[0]='\0';
    raw->sta.antsetup=raw->sta.itrf=raw->sta.deltype=0;
    for (i=0;i<3;i++) {
        raw->sta.pos[i]=raw->sta.del[i]=0.0;
    }
    raw->sta.hgt=0.0;
#endif
    return 1;

}






