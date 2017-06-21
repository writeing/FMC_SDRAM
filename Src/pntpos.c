/*------------------------------------------------------------------------------
* pntpos.c : standard positioning
*
*          Copyright (C) 2007-2010 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2010/07/28 1.0  moved from rtkcmn.c
*                           changed api:
*                               pntpos()
*                           deleted api:
*                               pntvel()
*           2011/01/12 1.1  add option to include unhealthy satellite
*                           reject duplicated observation data
*                           changed api: ionocorr()
*           2011/11/08 1.2  enable snr mask for single-mode (rtklib_2.4.1_p3)
*           2012/12/25 1.3  add variable snr mask
*-----------------------------------------------------------------------------*/

#include "rtklib.h"
#include <math.h>
#include <arm_math.h>
#include "global.h" 

//static const char rcsid[]="$Id:$";

/* constants -----------------------------------------------------------------*/

#define SQR(x)      ((x)*(x))

#define MAXITR      10          /* max number of iteration for point pos */
#define ERR_ION     5.0         /* ionospheric delay std (m) */
#define ERR_TROP    3.0         /* tropspheric delay std (m) */
#define ERR_SAAS    0.3         /* saastamoinen model error std (m) */
#define ERR_BRDCI   0.5         /* broadcast iono model error factor */
#define ERR_CBIAS   0.3         /* code bias error std (m) */
#define REL_HUMI    0.7         /* relative humidity for saastamoinen model */


/* pseudorange measurement error variance ------------------------------------*/
//#pragma CODE_SECTION(varerr,"sect_ECODE_II");
static double varerr(const prcopt_t *opt, double el, int sys)
{
    double fact,varr;
    fact=sys==SYS_GLO?EFACT_GLO:(sys==SYS_SBS?EFACT_SBS:EFACT_GPS);
    varr=SQR(opt->err[0])*(SQR(opt->err[1])+SQR(opt->err[2])/sin(el));
    if (opt->ionoopt==IONOOPT_IFLC) varr*=SQR(3.0); /* iono-free */
    return SQR(fact)*varr;
}
/* get tgd parameter (m) -----------------------------------------------------*/
//#pragma CODE_SECTION(gettgd,"sect_ECODE_II");
static double gettgd(int sat, const nav_t *nav)
{
    int i;
    for (i=0;i<nav->n;i++) {
        if (nav->eph[i].sat!=sat) continue;
        return CLIGHT*nav->eph[i].tgd[0];
    }
    return 0.0;
}
/* psendorange with code bias correction -------------------------------------*/
//#pragma CODE_SECTION(prange,"sect_ECODE_II");
static double prange(const obsd_t *obs, const nav_t *nav, const double *azel,
                     int iter, const prcopt_t *opt, double *var)
{
    const double *lam=nav->lam[obs->sat-1];
    double PC,P1,P2,P1_P2,P1_C1,P2_C2,gamma;
    int i=0,j=1,sys;
    
    *var=0.0;
    
    if (!(sys=satsys(obs->sat,NULL))) return 0.0;
    
    /* L1-L2 for GPS/GLO/QZS, L1-L5 for GAL/SBS */
    if (NFREQ>=3&&(sys&(SYS_GAL|SYS_SBS|SYS_CMP))) j=2;
    
    if (NFREQ<2||lam[i]==0.0||lam[j]==0.0) return 0.0;
    
    /* test snr mask */
    if (iter>0) {
        if (testsnr(0,i,azel[1],obs->SNR[i]*0.25,&opt->snrmask)) {
            //trace(4,"snr mask: %s sat=%2d el=%.1f snr=%.1f\n",
            //      time_str(obs->time,0),obs->sat,azel[1]*R2D,obs->SNR[i]*0.25);
//            LOG_printf(&trace,"snr mask sat=%d el=%d snr=%d",obs->sat,(int)(azel[1]*R2D*10),(int)(obs->SNR[i]*0.25));
            return 0.0;
        }
        if (opt->ionoopt==IONOOPT_IFLC) {
            if (testsnr(0,j,azel[1],obs->SNR[j]*0.25,&opt->snrmask)) return 0.0;
        }
    }
    gamma=SQR(lam[j])/SQR(lam[i]); /* f1^2/f2^2 */
    P1=obs->P[i];
    P2=obs->P[j];
    P1_P2=nav->cbias[obs->sat-1][0];
    P1_C1=nav->cbias[obs->sat-1][1];
    P2_C2=nav->cbias[obs->sat-1][2];
    
    /* if no P1-P2 DCB, use TGD instead */
    if (P1_P2==0.0&&(sys&(SYS_GPS|SYS_GAL|SYS_QZS))) {
        P1_P2=(1.0-gamma)*gettgd(obs->sat,nav);
    }
    if (opt->ionoopt==IONOOPT_IFLC) { /* dual-frequency */
        
        if (P1==0.0||P2==0.0) return 0.0;
        if (obs->code[i]==CODE_L1C) P1+=P1_C1; /* C1->P1 */
        if (obs->code[j]==CODE_L2C) P2+=P2_C2; /* C2->P2 */
        
        /* iono-free combination */
        PC=(gamma*P1-P2)/(gamma-1.0);
    }
    else { /* single-frequency */
        
        if (P1==0.0) return 0.0;
        if (obs->code[i]==CODE_L1C) P1+=P1_C1; /* C1->P1 */
        PC=P1-P1_P2/(1.0-gamma);
    }
    if (opt->sateph==EPHOPT_SBAS) PC-=P1_C1; /* sbas clock based C1 */
    
    *var=SQR(ERR_CBIAS);
    
    return PC;
}
/* ionospheric correction ------------------------------------------------------
* compute ionospheric correction
* args   : gtime_t time     I   time
*          nav_t  *nav      I   navigation data
*          int    sat       I   satellite number
*          double *pos      I   receiver position {lat,lon,h} (rad|m)
*          double *azel     I   azimuth/elevation angle {az,el} (rad)
*          int    ionoopt   I   ionospheric correction option (IONOOPT_???)
*          double *ion      O   ionospheric delay (L1) (m)
*          double *var      O   ionospheric delay (L1) variance (m^2)
* return : status(1:ok,0:error)
*-----------------------------------------------------------------------------*/
//#pragma CODE_SECTION(ionocorr,"sect_ECODE_II");
extern int ionocorr(gtime_t time, const nav_t *nav, int sat, const double *pos,
                    const double *azel, int ionoopt, double *ion, double *var)
{
    //trace(4,"ionocorr: time=%s opt=%d sat=%2d pos=%.3f %.3f azel=%.3f %.3f\n",
    //      time_str(time,3),ionoopt,sat,pos[0]*R2D,pos[1]*R2D,azel[0]*R2D,
    //      azel[1]*R2D);
    
    /* broadcast model */
    if (ionoopt==IONOOPT_BRDC) {
        *ion=ionmodel(time,nav->ion_gps,pos,azel);
        *var=SQR(*ion*ERR_BRDCI);
		//LOG_printf(&trace,"ion BRDC");
        return 1;
    }
    /* sbas ionosphere model */
    //if (ionoopt==IONOOPT_SBAS) {
    //    return sbsioncorr(time,nav,pos,azel,ion,var);
    //}
    /* ionex tec model */
    //if (ionoopt==IONOOPT_TEC) {
    //    return iontec(time,nav,pos,azel,1,ion,var);
    //}
    /* qzss broadcast model */
    if (ionoopt==IONOOPT_QZS&&norm(nav->ion_qzs,8)>0.0) {
        *ion=ionmodel(time,nav->ion_qzs,pos,azel);
        *var=SQR(*ion*ERR_BRDCI);
		//LOG_printf(&trace,"ion QZS");
        return 1;
    }
    /* lex ionosphere model */
    //if (ionoopt==IONOOPT_LEX) {
    //    return lexioncorr(time,nav,pos,azel,ion,var);
   // }
    *ion=0.0;
    *var=ionoopt==IONOOPT_OFF?SQR(ERR_ION):0.0;
	//LOG_printf(&trace,"no ion corr");
    return 1;
}
/* tropospheric correction -----------------------------------------------------
* compute tropospheric correction
* args   : gtime_t time     I   time
*          nav_t  *nav      I   navigation data
*          double *pos      I   receiver position {lat,lon,h} (rad|m)
*          double *azel     I   azimuth/elevation angle {az,el} (rad)
*          int    tropopt   I   tropospheric correction option (TROPOPT_???)
*          double *trp      O   tropospheric delay (m)
*          double *var      O   tropospheric delay variance (m^2)
* return : status(1:ok,0:error)
*-----------------------------------------------------------------------------*/
//#pragma CODE_SECTION(tropcorr,"sect_ECODE_II");
extern int tropcorr(gtime_t time, const nav_t *nav, const double *pos,
                    const double *azel, int tropopt, double *trp, double *var)
{
    //trace(4,"tropcorr: time=%s opt=%d pos=%.3f %.3f azel=%.3f %.3f\n",
    //      time_str(time,3),tropopt,pos[0]*R2D,pos[1]*R2D,azel[0]*R2D,
    //      azel[1]*R2D);
    
    /* saastamoinen model */
    if (tropopt==TROPOPT_SAAS||tropopt==TROPOPT_EST||tropopt==TROPOPT_ESTG) {
        *trp=tropmodel(time,pos,azel,REL_HUMI);
        *var=SQR(ERR_SAAS/(sin(azel[1])+0.1));
		//LOG_printf(&trace,"TROP corr");
        return 1;
    }
    /* sbas troposphere model */
    //if (tropopt==TROPOPT_SBAS) {
    //    *trp=sbstropcorr(time,pos,azel,var);
    //    return 1;
    //}
    /* no correction */
    *trp=0.0;
    *var=tropopt==TROPOPT_OFF?SQR(ERR_TROP):0.0;
	//LOG_printf(&trace,"no TROP corr");
    return 1;
}
/* pseudorange residuals -----------------------------------------------------*/
//#pragma CODE_SECTION(rescode,"sect_ECODE_I");
static int rescode(int iter, const obsd_t *obs, int n, const double *rs,
                   const double *dts, const double *vare, const int *svh,
                   const nav_t *nav, const double *x, const prcopt_t *opt,
                   double *v, double *H, double *var, double *azel, int *vsat,
                   double *resp, int *nx)
{
    double r,dion,dtrp,vmeas,vion,vtrp,rr[3],pos[3],dtr,e[3],P;
    int i,j,nv=0,ns[3]={0},sys;
    int nsx=0;//num of sys
    
    //trace(3,"resprng : n=%d\n",n);
    
    
    for (i=0;i<3;i++) rr[i]=x[i]; dtr=x[3];
    
    ecef2pos(rr,pos);
    
    for (i=0;i<n&&i<MAXOBS;i++) {
        vsat[i]=0; azel[i*2]=azel[1+i*2]=resp[i]=0.0;
        
        if (!(sys=satsys(obs[i].sat,NULL))) continue;
        
        /* reject duplicated observation data */
        if (i<n-1&&i<MAXOBS-1&&obs[i].sat==obs[i+1].sat) {
            //trace(2,"duplicated observation data %s sat=%2d\n",
            //time_str(obs[i].time,3),obs[i].sat);
            i++;
            continue;
        }
				
				r=geodist(rs+i*6,rr,e);
				if(r>0.0)
				{
					g_dElev[obs[i].sat-1] = satazel(pos,e,azel+i*2);
				}
        /* geometric distance/azimuth/elevation angle */
        if ((r=geodist(rs+i*6,rr,e))<=0.0||
            satazel(pos,e,azel+i*2)<opt->elmin) continue;
        
        /* psudorange with code bias correction */
        if ((P=prange(obs+i,nav,azel+i*2,iter,opt,&vmeas))==0.0) continue;
        
        /* excluded satellite? */
        if (satexclude(obs[i].sat,svh[i],opt)) continue;
        
        /* ionospheric corrections */
        if (!ionocorr(obs[i].time,nav,obs[i].sat,pos,azel+i*2,
                      iter>0?opt->ionoopt:IONOOPT_BRDC,&dion,&vion)) continue;
        
        /* tropospheric corrections */
        if (!tropcorr(obs[i].time,nav,pos,azel+i*2,
                      iter>0?opt->tropopt:TROPOPT_SAAS,&dtrp,&vtrp)) {
            continue;
        }
        /* pseudorange residual */
        v[nv]=P-(r+dtr-CLIGHT*dts[i*2]+dion+dtrp);
        
        /* design matrix */
        for (j=0;j<4;j++) H[j+nv*6]=j<3?-e[j]:1.0;
        
        /* time system and receiver bias offset */
		if (sys==SYS_GPS){						H[4+nv*6]=0.0; H[5+nv*6]=0.0; ns[0]++;}
		if (sys==SYS_GLO){v[nv]-=x[4];			H[4+nv*6]=1.0; H[5+nv*6]=0.0; ns[1]++;}
		if (sys==SYS_CMP){v[nv]-=(x[4]+x[5]);		H[4+nv*6]=1.0; H[5+nv*6]=1.0; ns[2]++;}
     
        vsat[i]=1; resp[i]=v[nv];
        
        /* error variance */
        var[nv++]=varerr(opt,azel[1+i*2],sys)+vare[i]+vmeas+vion+vtrp;
        
        //trace(4,"sat=%2d azel=%5.1f %4.1f res=%7.3f sig=%5.3f\n",obs[i].sat,
        //      azel[i*2]*R2D,azel[1+i*2]*R2D,resp[i],sqrt(var[nv-1]));
    }
    /* shrink design matrix: nx=6->5->4 */
	*nx=0;
	nsx = (0!=ns[0])+(0!=ns[1])+(0!=ns[2]);	//count the number of systems
	if(1==nsx){	//there is only one system
        	for (i=0;i<nv;i++) for (j=0;j<4;j++) H[j+i*4]=H[j+i*6];
	 		*nx=4;
    }	
	if(2==nsx){	//there are two systems
		if(!ns[0]){	//lack of GPS  there still is bug
			for (i=0;i<nv;i++){
				for (j=0;j<4;j++) H[j+i*5]=H[j+i*6];
				H[4+i*5]=H[5+i*6]; }
		}
		else{ for (i=0;i<nv;i++)  for (j=0;j<5;j++) H[j+i*5]=H[j+i*6];}	//lack of CMP or GLO
		*nx=5;
	}
	if(3==nsx)	 *nx=6; //there are three systems
	//When 3 systems be,there are 7 combinations that are {(G+R+C),(G+R),(G+C),(R+C),(G),(R),(C)}
	//All of them are right except in the case of(R+C)
	return nv;
}

/* pseudorange residuals -----------------------------------------------------*/
//#pragma CODE_SECTION(rescode_three,"sect_ECODE_II");
static int rescode_three(int iter, const obsd_t *obs, int n, const double *rs,
                   const double *dts, const double *vare, const int *svh,
                   const nav_t *nav, const double *x, const prcopt_t *opt,
                   double *v, double *H, double *var, double *azel, int *vsat,
                   double *resp, int *nx)
{
    double r,dion,dtrp,vmeas,vion,vtrp,rr[3],pos[3],dtr,e[3],P;
    int i,j,nv=0,ns[3]={0},sys;
    
    //trace(3,"resprng : n=%d\n",n);
    
    *nx=6;
    
    for (i=0;i<3;i++) rr[i]=x[i]; dtr=x[3];
    
    ecef2pos(rr,pos);
    	
    for (i=0;i<n&&i<MAXOBS;i++) {
        vsat[i]=0; azel[i*2]=azel[1+i*2]=resp[i]=0.0;
        
        if (!(sys=satsys(obs[i].sat,NULL))) continue;
        
        // reject duplicated observation data //
        if (i<n-1&&i<MAXOBS-1&&obs[i].sat==obs[i+1].sat) {
            //trace(2,"duplicated observation data %s sat=%2d\n",
            //      time_str(obs[i].time,3),obs[i].sat);
//            LOG_printf(&trace,"duplicated obs sat=%d",obs[i].sat);
            i++;
            continue;
        }
        // geometric distance/azimuth/elevation angle //
        if ((r=geodist(rs+i*6,rr,e))<=0.0||
            satazel(pos,e,azel+i*2)<opt->elmin) continue;
        
        // psudorange with code bias correction //
        if ((P=prange(obs+i,nav,azel+i*2,iter,opt,&vmeas))==0.0) continue;
        
        // excluded satellite? //
        if (satexclude(obs[i].sat,svh[i],opt)) continue;
        
        // ionospheric corrections //
        if (!ionocorr(obs[i].time,nav,obs[i].sat,pos,azel+i*2,
                      iter>0?opt->ionoopt:IONOOPT_BRDC,&dion,&vion)) continue;
        //LOG_printf(&trace,"sat %d dion %d",obs[i].sat,(int)(dion*10));
        // tropospheric corrections //
        if (!tropcorr(obs[i].time,nav,pos,azel+i*2,
                      iter>0?opt->tropopt:TROPOPT_SAAS,&dtrp,&vtrp)) {
            continue;
        }
		//LOG_printf(&trace,"sat %d dtrp %d",obs[i].sat,(int)(dtrp*10));
		
        // pseudorange residual //
        v[nv]=P-(r+dtr-CLIGHT*dts[i*2]+dion+dtrp);
        
        // design matrix //
/*        for (j=0;j<4;j++) H[j+nv*5]=j<3?-e[j]:1.0;

	if(sys==SYS_GPS)
	{
		rtk_gps_use_num++;
	}
        // time system and receiver bias offset //
        if (sys==SYS_GLO||sys==SYS_CMP) 
	{
		v[nv]-=x[4]; H[4+nv*5]=1.0; ns[1]++;

		if(sys==SYS_GLO)
		{
			rtk_glo_use_num++;
			//LOG_printf(&trace,"GLO in martex");
		}
		else if(sys==SYS_CMP)
		{
			rtk_bd_use_num++;
			//LOG_printf(&trace,"BD in martex");
		}
	}
        else              {             H[4+nv*5]=0.0; ns[0]++;}
*/
	//---guan 201310.06------
        // design matrix //
        for (j=0;j<4;j++) H[j+nv*6]=j<3?-e[j]:1.0;


	if (sys==SYS_GLO) 
	{
		if(ns[0]>0)
		{
			v[nv]-=x[4]; 
			H[4+nv*6]=1.0; 
			H[5+nv*6]=0.0; 
		}
		else if(ns[0]==0)
		{
			//v[nv]-=x[4]; 
			H[4+nv*6]=0.0; 
			H[5+nv*6]=0.0;
		}
		ns[1]++;
		//LOG_printf(&trace,"GLO in martex");

		
	}
	else if(sys==SYS_CMP)
	{
		if((ns[0]>0)&&(ns[1]>0))
		{
			v[nv]-=x[5]; 
			H[4+nv*6]=0.0;
			H[5+nv*6]=1.0;
		}
		else if(((ns[0]>0)&&(ns[1]==0))||
			((ns[0]==0)&&(ns[1]>0)))
		{
			v[nv]-=x[4]; 
			H[4+nv*6]=1.0;
			H[5+nv*6]=0.0;
		}
		else if((ns[0]==0)&&(ns[1]==0))
		{
			//v[nv]-=x[4]; 
			H[4+nv*6]=0.0;
			H[5+nv*6]=0.0;
		}
		ns[2]++;

		//LOG_printf(&trace,"BD in martex");

	}
        else              
	{             
		H[4+nv*6]=0.0; 
		H[5+nv*6]=0.0;
		ns[0]++;

		//if(sys==SYS_GPS)
		{
			//rtk_gps_use_num++;
		}
	}
        //-----------------------------------------------
        vsat[i]=1; resp[i]=v[nv];
        
        //error variance //
        var[nv++]=varerr(opt,azel[1+i*2],sys)+vare[i]+vmeas+vion+vtrp;
        
        //trace(4,"sat=%2d azel=%5.1f %4.1f res=%7.3f sig=%5.3f\n",obs[i].sat,
        //      azel[i*2]*R2D,azel[1+i*2]*R2D,resp[i],sqrt(var[nv-1]));
    }//end for (i=0;i<n&&i<MAXOBS;i++)

	
    //shrink design matrix: nx=5->4 //
/*    if (!ns[0]||!ns[1]) {
        for (i=0;i<nv;i++) for (j=0;j<4;j++) H[j+i*4]=H[j+i*5];
        *nx=4;
    }*/
	//---guan 201310.06------
	if(((ns[0]>0)&&(ns[1]>0)&&(ns[2]==0))||
		((ns[0]>0)&&(ns[1]==0)&&(ns[2]>0)))
	{
		*nx=5;
		//for (i=0;i<nv;i++) for (j=0;j<5;j++) H[j+i*5]=H[j+i*6];
	}
	else if(((ns[0]>0)&&(ns[1]==0)&&(ns[2]==0))||
		((ns[0]==0)&&(ns[1]>0)&&(ns[2]==0))||
		((ns[0]==0)&&(ns[1]==0)&&(ns[2]>0)))
	{
		*nx=4;
		//for (i=0;i<nv;i++) for (j=0;j<4;j++) H[j+i*4]=H[j+i*6];
	}
	else if((ns[0]==0)&&(ns[1]>0)&&(ns[2]>0))
	{
		*nx=MAXOBS+1;
	}
	else if((ns[0]==0)&&(ns[1]==0)&&(ns[2]==0))
	{
		*nx=MAXOBS+1;
	}

	if((*nx==4)||(*nx==5))
	{
		for (i=0;i<nv;i++) for (j=0;j<*nx;j++) H[j+i**nx]=H[j+i*6];
	}
	//-----------------------------------
    return nv;
}

/* validate solution ---------------------------------------------------------*/
//#pragma CODE_SECTION(valsol,"sect_ECODE_II");
static int valsol(const double *azel, const int *vsat, int n,
                  const prcopt_t *opt, const double *v, int nv, int nx,
                  char *msg)
{
    double azels[MAXOBS*2],dop[4],vv;
    int i,ns;
    
    //trace(3,"valsol  : n=%d nv=%d\n",n,nv);
    
    /* chi-square validation of residuals */
    vv=dot(v,v,nv);
    if (nv>nx&&vv>chisqr[nv-nx-1]) {
        //sprintf(msg,"chi-square error nv=%d vv=%.1f cs=%.1f",nv,vv,chisqr[nv-nx-1]);
        return 0;
    }
    /* large gdop check */
    for (i=ns=0;i<n;i++) {
        if (!vsat[i]) continue;
        azels[  ns*2]=azel[  i*2];
        azels[1+ns*2]=azel[1+i*2];
        ns++;
    }
    dops(ns,azels,opt->elmin,dop);
    if (dop[0]<=0.0||dop[0]>opt->maxgdop) {
        //sprintf(msg,"gdop error nv=%d gdop=%.1f",nv,dop[0]);
        return 0;
    }
    return 1;
}
/* estimate receiver position ------------------------------------------------*/
//#pragma CODE_SECTION(estpos,"sect_ECODE_I");
//#pragma DATA_SECTION(dMatV,"sect_EDATA_I");
//#pragma DATA_SECTION(dMatH,"sect_EDATA_I");
//#pragma DATA_SECTION(dMatVar,"sect_EDATA_I");
double dMatV[90]={0.0},dMatH[630]={0.0},dMatVar[90]={0.0};

static int estpos(const obsd_t *obs, int n, const double *rs, const double *dts,
                  const double *vare, const int *svh, const nav_t *nav,
                  const prcopt_t *opt, sol_t *sol, double *azel, int *vsat,
                  double *resp, char *msg)
{
    double x[7]={0},dx[7],Q[49],*v,*H,*var,sig;
    int i,j,k,info,stat,nx,nv;
    
    //trace(3,"estpos  : n=%d\n",n);
    
    //v=mat(n,1); H=mat(7,n); var=mat(n,1);

	v=dMatV; H=dMatH; var=dMatVar;
    
    for (i=0;i<3;i++) x[i]=sol->rr[i];
    //LOG_printf(&trace,"start estpos");
    for (i=0;i<MAXITR;i++) {
        
        /* pseudorange residuals */
        //nv=rescode(i,obs,n,rs,dts,vare,svh,nav,x,opt,v,H,var,azel,vsat,resp,&nx);
	 nv=rescode(i,obs,n,rs,dts,vare,svh,nav,x,opt,v,H,var,azel,vsat,resp,&nx);
	if(nv<=0)
		{
		//free(v); free(H); free(var);
		 return 0;}
		
        if (nv<abs(nx)) {
            //sprintf(msg,"lack of valid sats ns=%d",nv);
//            LOG_printf(&trace,"lack of valid sats ns=%d",nv);
            break;
        }
		if (nx<0) {
            //sprintf(msg,"lack of valid sats ns=%d",nv);
//            LOG_printf(&trace,"lack of valid sats ns=%d",nv);
            break;
        }
		if (nx>6) {
            //sprintf(msg,"lack of valid sats ns=%d",nv);
//            LOG_printf(&trace,"lack of valid sats ns=%d",nv);
            break;
        }
        /* weight by variance */
        for (j=0;j<nv;j++) {
            sig=sqrt(var[j]);
            v[j]/=sig;
            for (k=0;k<nx;k++) H[k+j*nx]/=sig;
        }
        /* least square estimation */
        if ((info=lsq(H,v,nx,nv,dx,Q))) {
            //sprintf(msg,"lsq error info=%d",info);
//            LOG_printf(&trace,"lsq error info=%d",info);
            break;
        }
        for (j=0;j<nx;j++) x[j]+=dx[j];
        
        if (norm(dx,nx)<1E-4) {
            sol->type=0;
            sol->time=timeadd(obs[0].time,-x[3]/CLIGHT);
            sol->dtr[0]=x[3]/CLIGHT; /* receiver clock bias (s) */
            sol->dtr[1]=x[4]/CLIGHT; /* glonass-gps time offset (s) */
            sol->dtr[2]=(x[4]+x[5])/CLIGHT;/* BD-gps time offset (s) 2013.11.12*/
            for (j=0;j<6;j++) sol->rr[j]=j<3?x[j]:0.0;
            for (j=0;j<3;j++) sol->qr[j]=(float)Q[j+j*nx];
            sol->qr[3]=(float)Q[1];    /* cov xy */
            sol->qr[4]=(float)Q[2+nx]; /* cov yz */
            sol->qr[5]=(float)Q[2];    /* cov zx */
            sol->ns=(unsigned char)nv;
            sol->age=sol->ratio=0.0;
            
            /* validate solution */
            if ((stat=valsol(azel,vsat,n,opt,v,nv,nx,msg))) {
                sol->stat=opt->sateph==EPHOPT_SBAS?SOLQ_SBAS:SOLQ_SINGLE;
            }
            //free(v); free(H); free(var);
             //LOG_printf(&trace,"stat %d",stat);
             //LOG_printf(&trace,"norm less 1e-4");
            return stat;
        }//end if (norm(dx,nx)<1E-4)
        //else 
        //{
	//	LOG_printf(&trace,"norm more 1e-4");
	//  }
    }//end for (i=0;i<MAXITR;i++)
    //if (i>=MAXITR) sprintf(msg,"iteration divergent i=%d",i);
    
//    free(v); free(H); free(var);
    
    return 0;
}
/* raim fde (failure detection and exclution) -------------------------------*/
//#pragma CODE_SECTION(raim_fde,"sect_ECODE_II");
//#pragma CODE_SECTION(estpos,"sect_ECODE_II");
//#pragma DATA_SECTION(dMat_rs_e,"sect_EDATA_I");
//#pragma DATA_SECTION(dMat_dts_e,"sect_EDATA_I");
//#pragma DATA_SECTION(dMat_vare_e,"sect_EDATA_I");
//#pragma DATA_SECTION(dMat_azel_e,"sect_EDATA_I");
//#pragma DATA_SECTION(dMat_resp_e,"sect_EDATA_I");
//#pragma DATA_SECTION(iMat_svh_e,"sect_EDATA_I");
//#pragma DATA_SECTION(iMat_vsat_e,"sect_EDATA_I");
double dMat_rs_e[300]={0.0},dMat_dts_e[100]={0.0},dMat_vare_e[50]={0.0},dMat_azel_e[100]={0.0},dMat_resp_e[50]={0.0};
int iMat_svh_e[50]={0},iMat_vsat_e[50]={0};
obsd_t obs_raim[50];
static int raim_fde(const obsd_t *obs, int n, const double *rs,
                    const double *dts, const double *vare, const int *svh,
                    const nav_t *nav, const prcopt_t *opt, sol_t *sol,
                    double *azel, int *vsat, double *resp, char *msg)
{
    obsd_t *obs_e;
    sol_t sol_e={{0}};
    char tstr[32],name[16],msg_e[128];
    double *rs_e,*dts_e,*vare_e,*azel_e,*resp_e,rms_e,rms=100.0;
    int i,j,k,nvsat,stat=0,*svh_e,*vsat_e,sat=0;
    
    //trace(3,"raim_fde: %s n=%2d\n",time_str(obs[0].time,0),n);
//    LOG_printf(&trace,"raim_fde");
    //if (!(obs_e=(obsd_t *)malloc(sizeof(obsd_t)*n))) return 0;

	obs_e = &obs_raim[0];

    //rs_e = mat(6,n); dts_e = mat(2,n); vare_e=mat(1,n); azel_e=zeros(2,n);
    //svh_e=imat(1,n); vsat_e=imat(1,n); resp_e=mat(1,n); 

	memset(dMat_azel_e,0,sizeof(dMat_azel_e));
	rs_e = dMat_rs_e; dts_e = dMat_dts_e; vare_e=dMat_vare_e; azel_e=dMat_azel_e;
    svh_e=iMat_svh_e; vsat_e=iMat_vsat_e; resp_e=dMat_resp_e;
    
    for (i=0;i<n;i++) {
        
        /* satellite exclution */
        for (j=k=0;j<n;j++) {
            if (j==i) continue;
            obs_e[k]=obs[j];
            matcpy(rs_e +6*k,rs +6*j,6,1);
            matcpy(dts_e+2*k,dts+2*j,2,1);
            vare_e[k]=vare[j];
            svh_e[k++]=svh[j];
        }
        /* estimate receiver position without a satellite */
        if (!estpos(obs_e,n-1,rs_e,dts_e,vare_e,svh_e,nav,opt,&sol_e,azel_e,
                    vsat_e,resp_e,msg_e)) {
            //trace(3,"raim_fde: exsat=%2d (%s)\n",obs[i].sat,msg);
            continue;
        }
        for (j=nvsat=0,rms_e=0.0;j<n-1;j++) {
            if (!vsat_e[j]) continue;
            rms_e+=SQR(resp_e[j]);
            nvsat++;
        }
        if (nvsat<5) {
            //trace(3,"raim_fde: exsat=%2d lack of satellites nvsat=%2d\n",
            //      obs[i].sat,nvsat);
            continue;
        }
        rms_e=sqrt(rms_e/nvsat);
        
        //trace(3,"raim_fde: exsat=%2d rms=%8.3f\n",obs[i].sat,rms_e);
        
        if (rms_e>rms) continue;
        
        /* save result */
        for (j=k=0;j<n;j++) {
            if (j==i) continue;
            matcpy(azel+2*j,azel_e+2*k,2,1);
            vsat[j]=vsat_e[k];
            resp[j]=resp_e[k++];
        }
        stat=1;
        *sol=sol_e;
        sat=obs[i].sat;
        rms=rms_e;
        vsat[i]=0;
        //strcpy(msg,msg_e);
    }
    if (stat) {
        //time2str(obs[0].time,tstr,2); satno2id(sat,name);
        //trace(2,"%s: %s excluded by raim\n",tstr+11,name);
    }
    //free(obs_e);
    //free(rs_e ); free(dts_e ); free(vare_e); free(azel_e);
    //free(svh_e); free(vsat_e); free(resp_e);
    return stat;
}
/* doppler residuals ---------------------------------------------------------*/
//#pragma CODE_SECTION(resdop,"sect_ECODE_II");
static int resdop(const obsd_t *obs, int n, const double *rs, const double *dts,
                  const nav_t *nav, const double *rr, const double *x,
                  const double *azel, const int *vsat, double *v, double *H)
{
    double lam,rate,pos[3],E[9],a[3],e[3],vs[3],cosel;
    int i,j,nv=0;
    
   // trace(3,"resdop  : n=%d\n",n);
    
    ecef2pos(rr,pos); xyz2enu(pos,E);
    
    for (i=0;i<n&&i<MAXOBS;i++) {
        
        lam=nav->lam[obs[i].sat-1][0];
        
        if (obs[i].D[0]==0.0||lam==0.0||!vsat[i]||norm(rs+3+i*6,3)<=0.0) {
            continue;
        }
        /* line-of-sight vector in ecef */
        cosel=cos(azel[1+i*2]);
        a[0]=sin(azel[i*2])*cosel;
        a[1]=cos(azel[i*2])*cosel;
        a[2]=sin(azel[1+i*2]);
        matmul("TN",3,1,3,1.0,E,a,0.0,e);
        
        /* satellite velocity relative to receiver in ecef */
        for (j=0;j<3;j++) vs[j]=rs[j+3+i*6]-x[j];
        
        /* range rate with earth rotation correction */
        rate=dot(vs,e,3)+OMGE/CLIGHT*(rs[4+i*6]*rr[0]+rs[1+i*6]*x[0]-
                                      rs[3+i*6]*rr[1]-rs[  i*6]*x[1]);
        
        /* doppler residual */
        v[nv]=-lam*obs[i].D[0]-(rate+x[3]-CLIGHT*dts[1+i*2]);
        
        /* design matrix */
        for (j=0;j<4;j++) H[j+nv*4]=j<3?-e[j]:1.0;
        
        nv++;
    }
    return nv;
}
/* estimate receiver velocity ------------------------------------------------*/
//#pragma CODE_SECTION(estvel,"sect_ECODE_II");
//#pragma DATA_SECTION(dMat_v3,"sect_EDATA_I");
//#pragma DATA_SECTION(dMat_H3,"sect_EDATA_I");
double dMat_v3[50]={0.0},dMat_H3[300]={200};

static void estvel(const obsd_t *obs, int n, const double *rs, const double *dts,
                   const nav_t *nav, const prcopt_t *opt, sol_t *sol,
                   const double *azel, const int *vsat)
{
    double x[4]={0},dx[4],Q[16],*v,*H;
    int i,j,nv;
    
    //trace(3,"estvel  : n=%d\n",n);
//    LOG_printf(&trace,"estvel");
    //v=mat(n,1); H=mat(4,n);
	v=dMat_v3; H=dMat_H3;
    
    for (i=0;i<MAXITR;i++) {
        
        /* doppler residuals */
        if ((nv=resdop(obs,n,rs,dts,nav,sol->rr,x,azel,vsat,v,H))<4) {
            break;
        }
        /* least square estimation */
        if (lsq(H,v,4,nv,dx,Q)) break;
        
        for (j=0;j<4;j++) x[j]+=dx[j];
        
        if (norm(dx,4)<1E-6) {
            for (i=0;i<3;i++) sol->rr[i+3]=x[i];
            break;
        }
    }
    //free(v); free(H);
}
/* single-point positioning ----------------------------------------------------
* compute receiver position, velocity, clock bias by single-point positioning
* with pseudorange and doppler observables
* args   : obsd_t *obs      I   observation data
*          int    n         I   number of observation data
*          nav_t  *nav      I   navigation data
*          prcopt_t *opt    I   processing options
*          sol_t  *sol      IO  solution
*          double *azel     IO  azimuth/elevation angle (rad) (NULL: no output)
*          ssat_t *ssat     IO  satellite status              (NULL: no output)
*          char   *msg      O   error message for error exit
* return : status(1:ok,0:error)
* notes  : assuming sbas-gps, galileo-gps, qzss-gps, compass-gps time offset and
*          receiver bias are negligible (only involving glonass-gps time offset
*          and receiver bias)
*-----------------------------------------------------------------------------*/

//#pragma CODE_SECTION(pntpos,"sect_ECODE_II");
//#pragma DATA_SECTION(dMat_rs,"sect_EDATA_I");
//#pragma DATA_SECTION(dMat_dts,"sect_EDATA_I");
//#pragma DATA_SECTION(dMat_var,"sect_EDATA_I");
//#pragma DATA_SECTION(dMat_azel_,"sect_EDATA_I");
//#pragma DATA_SECTION(dMat_resp,"sect_EDATA_I");
//#pragma DATA_SECTION(opt_,"sect_EDATA_I");
prcopt_t opt_;
double dMat_rs[300]={0.0},dMat_dts[100]={0.0},dMat_var[50]={0.0},dMat_azel_[100]={0.0},dMat_resp[50]={0.0};

extern int pntpos(const obsd_t *obs, int n, const nav_t *nav,
                  const prcopt_t *opt, sol_t *sol, double *azel, ssat_t *ssat,
                  char *msg)
{
    //prcopt_t opt_=*opt;
    double *rs,*dts,*var,*azel_,*resp;
    int i,stat,vsat[MAXOBS]={0},svh[MAXOBS];
    
    sol->stat=SOLQ_NONE;
	opt_=*opt;
    
    if (n<=0) {//strcpy(msg,"no observation data"); 
    return 0;}
    
//    LOG_printf(&trace,"single start");
    sol->time=obs[0].time; //msg[0]='\0';
    
    //rs=mat(6,n); dts=mat(2,n); var=mat(1,n); azel_=zeros(2,n); resp=mat(1,n);
 
	memset(dMat_azel_,0,sizeof(dMat_azel_));
	rs=dMat_rs; dts=dMat_dts; var=dMat_var; azel_=dMat_azel_; resp=dMat_resp;
 //return 0;   
    if (opt_.mode!=PMODE_SINGLE) { /* for precise positioning */
#if 0
        opt_.sateph =EPHOPT_BRDC;
#endif
        opt_.ionoopt=IONOOPT_BRDC;
        opt_.tropopt=TROPOPT_SAAS;
    }
    /* satellite positons, velocities and clocks */
    satposs(sol->time,obs,n,nav,opt_.sateph,rs,dts,var,svh);
   
    /* estimate receiver position with pseudorange */
    stat=estpos(obs,n,rs,dts,var,svh,nav,&opt_,sol,azel_,vsat,resp,msg);
//    LOG_printf(&trace,"stat %d",stat);
    /* raim fde */
    if (!stat&&n>=6&&opt->posopt[4]) {
        stat=raim_fde(obs,n,rs,dts,var,svh,nav,&opt_,sol,azel_,vsat,resp,msg);
    }
    /* estimate receiver velocity with doppler */
    if (stat) estvel(obs,n,rs,dts,nav,&opt_,sol,azel_,vsat);
    
    if (azel) {
        for (i=0;i<n*2;i++) azel[i]=azel_[i];
    }
    if (ssat) {
        for (i=0;i<MAXSAT;i++) {
            ssat[i].vs=0;
            ssat[i].azel[0]=ssat[i].azel[1]=0.0;
            ssat[i].resp[0]=ssat[i].resc[0]=0.0;
            ssat[i].snr[0]=0;
        }
        for (i=0;i<n;i++) {
            ssat[obs[i].sat-1].azel[0]=azel_[  i*2];
            ssat[obs[i].sat-1].azel[1]=azel_[1+i*2];
            ssat[obs[i].sat-1].snr[0]=obs[i].SNR[0];
            if (!vsat[i]) continue;
            ssat[obs[i].sat-1].vs=1;
            ssat[obs[i].sat-1].resp[0]=resp[i];
        }
    }
    //free(rs); free(dts); free(var); free(azel_); free(resp);
    return stat;
}
