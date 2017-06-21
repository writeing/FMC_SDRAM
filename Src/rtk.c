//#include <std.h>
#include <stdio.h>
//#include <c6x.h>
#include "global.h"
//#include "HInclude.h"
#include "MyConst.h"
#include "rtkpnt.h"
#include "rtklib.h"
#include "rtk.h" 

//#include "DynamicAttiLib.h"
#include <stdlib.h>

PntOpt_t Opt_t;
unsigned int gCalcRtkUseTime =0;

//extern geph_t geph[NSATGLO]; 

///void BaseRover_AttiStruct( TDIFFOBSINFOSTRUCT *ptBaseSat, TDIFFOBSINFOSTRUCT *ptRoverSat, TATTITUDESTRUCT *ptAttitudeStruct );
//////////////////////////////////////////////////////////////////////////
///////////////////////////////RTK参数初始化//////////////////////////////
////////////////////////////////////////////////////////////////////////// 
//#pragma CODE_SECTION(RtkParaInit,"sect_ECODE_II");
void RtkParaInit( void )
{
	memset(&g_tRtk,0,sizeof(g_tRtk));
	memset(&obs,0,sizeof(obs));
	memset(&data,0,sizeof(data));
	memset(&dataBase,0,sizeof(dataBase));
	memset(&nav,0,sizeof(nav));
	memset(&eph,0,sizeof(eph));
	memset(&ephBase,0,sizeof(ephBase));
	memset(g_dElev,0,sizeof(g_dElev));
	memset(&g_tSatElev,0,sizeof(g_tSatElev));
	memset(g_dDop,0,sizeof(g_dDop));
	
	
	
	//memset(&rtk_result,0,sizeof(rtk_result));
	memset(g_usCommonSat,0,sizeof(g_usCommonSat));

	memset(&g_tRawBase,0,sizeof(g_tRawBase));
	memset(&g_tRawRover,0,sizeof(g_tRawRover));
	
	memset(&g_tBdUtcTimePara,0,sizeof(g_tBdUtcTimePara));
	memset(&g_tGpsUtcTimePara,0,sizeof(g_tGpsUtcTimePara));
	memset(&g_tUartRmo,0,sizeof(g_tUartRmo));
	g_tUartRmo.GGA = 0;
	g_tUartRmo.ZDA = 0;
	g_tUartRmo.RTK = 0;
	g_tUartRmo.BIN = 0;

	g_tRawRover.obs.data=&data[0];
	g_tRawRover.nav.eph = &eph[0];

	g_tRawBase.obs.data=&dataBase[0];
	g_tRawBase.nav.eph = &ephBase[0];
	//init_raw(&g_tRawBase);
	//init_raw(&g_tRawRover);
	
	
} 

//////////////////////////////////////////////////////////////////////////
/////////////////////////获取RTK格式的BASE观测量//////////////////////////
//////////////////////////////////////////////////////////////////////////  
unsigned int uiPseudoErrCount = 0;
void FormDiffObs( raw_t *ptRaw,TDIFFOBSINFOSTRUCT *ptDiffObs )
{
	unsigned short i,usSvID,usSatIndex=0;
	int iGpsWeek;
	double dGpsSecond;

	dGpsSecond=time2gpst(ptRaw->time,&iGpsWeek);
	
	ptDiffObs->usWeek								= iGpsWeek;
	ptDiffObs->dSecond								= dGpsSecond;
	for (i=0;i<ptRaw->obs.n;i++)
	{
		usSvID	= ptRaw->obs.data[i].sat;
		
		if (ptRaw->obs.data[i].LLI[0]>0)
			continue;
		if (usSvID>46)//北斗的有一颗星暂时不用
			//和兴和思南,glonass做到了rtk,说是买了老外的技术,但是其他大家都觉得glonass信号做rtk不好
		//glonass的频率很高,伪码精度很高
		//伪距没有平滑,抖动很大,能到2~3m
		//rtk核心思想是整周模糊度求出来
			continue;
		
						
		/*if(ptRaw->obs.data[i].L[0]<0.02*1.57542e9)
		{
			uiPseudoErrCount++;
			//continue;
		}
		
		if(ptRaw->obs.data[i].L[0]>0.3*1.57542e9)
		{
			uiPseudoErrCount++;
			//continue;
		}*/
			
		ptDiffObs->usSvID[usSatIndex]				= usSvID;
		
		ptDiffObs->dCarrierPhase[usSatIndex][0] 	= ptRaw->obs.data[i].L[0]; // B1频点的载波周
		ptDiffObs->dCarrierPhase[usSatIndex][1] 	= 0;
		ptDiffObs->dCarrierPhase[usSatIndex][2] 	= 0; // B3频点的载波周
		
		ptDiffObs->dPseudoRange[usSatIndex][0] 		= ptRaw->obs.data[i].P[0]; // B1频点的载波周
		ptDiffObs->dPseudoRange[usSatIndex][1] 		= 0; // B2频点的载波周
		ptDiffObs->dPseudoRange[usSatIndex][2] 		= 0; // B3频点的载波周
		
		ptDiffObs->ucLostFlag[usSatIndex][0]		= ptRaw->obs.data[i].LLI[0]; // 0:未失锁
		ptDiffObs->ucLostFlag[usSatIndex][1]		= 1; // 0:失锁
		ptDiffObs->ucLostFlag[usSatIndex][2]		= 1; // 0:失锁
		
		ptDiffObs->fDoppler[usSatIndex][0]			= ptRaw->obs.data[i].D[0];
		ptDiffObs->fDoppler[usSatIndex][1]			= 0;
		ptDiffObs->fDoppler[usSatIndex][2]			= 0;
		
		ptDiffObs->ucSnr[usSatIndex][0]				= ptRaw->obs.data[i].SNR[0]/4; // B1频点的信噪比
		ptDiffObs->ucSnr[usSatIndex][1]				= 0; // B2频点的信噪比
		ptDiffObs->ucSnr[usSatIndex][2]				= 0; // B3频点的信噪比
		usSatIndex++;
		
	}
	ptDiffObs->usNsats	= usSatIndex;
} 

//////////////////////////////////////////////////////////////////////////
/////////////////////////获取RTK格式的ROVER观测量/////////////////////////
//////////////////////////////////////////////////////////////////////////   
//#pragma CODE_SECTION(FormRtkRoverObs,"sect_ECODE_II");
void FormRtkRoverObs( TDIFFOBSINFOSTRUCT *ptDiffObs )
{
	unsigned short 	i,j,usSvID;
	gtime_t 		gps_time;

 	obs.data=&data[0];


	gps_time		= gpst2time(ptDiffObs->usWeek,ptDiffObs->dSecond);
	
	
	for(i=0;i<ptDiffObs->usNsats;i++)
	{
		usSvID = ptDiffObs->usSvID[i];// - 32;
		for(j=0;j<NFREQ+NEXOBS;j++)
		{
			obs.data[i].SNR[j]		= 0;
			obs.data[i].LLI[j]		= 1;
			obs.data[i].code[j]		= 0;
			obs.data[i].L[j]		= 0;
			obs.data[i].P[j]		= 0;
			obs.data[i].D[j]		= 0;
		}
		obs.data[i].time			= gps_time;

		if(usSvID>32)
		{
			obs.data[i].sat				= usSvID-32+NSATGPS+NSATGLO+NSATGAL+NSATQZS;
		}
		else
		{
			obs.data[i].sat				= usSvID;
		}
		// rcv类型，rover:1,base:2
		obs.data[i].rcv				=1;

		for(j=0;j<NFREQ+NEXOBS;j++)
		{

			obs.data[i].SNR[j]	= ptDiffObs->ucSnr[i][j];
			obs.data[i].LLI[j]	= ptDiffObs->ucLostFlag[i][j];
			
			obs.data[i].code[j]	= 0;
			obs.data[i].L[j]	= ptDiffObs->dCarrierPhase[i][j];
			obs.data[i].P[j]	= ptDiffObs->dPseudoRange[i][j];

			obs.data[i].D[j]	= ptDiffObs->fDoppler[i][j];
			
		}

	}
	obs.n		= ptDiffObs->usNsats;
	obs.nmax	= 0;

} 

//////////////////////////////////////////////////////////////////////////
/////////////////////////获取RTK格式的BASE观测量//////////////////////////
//////////////////////////////////////////////////////////////////////////  
//#pragma CODE_SECTION(FormRtkBaseObs,"sect_ECODE_II");
void FormRtkBaseObs( TDIFFOBSINFOSTRUCT *ptDiffObs )
{
	unsigned short i,j,usSvID,usChOffSet = 0;
	gtime_t gps_time;

	usChOffSet=obs.n;


	gps_time		= gpst2time(ptDiffObs->usWeek,ptDiffObs->dSecond);


		
	for(i=0;i<ptDiffObs->usNsats;i++)
	{
		usSvID	= ptDiffObs->usSvID[i];
		for(j=0;j<NFREQ+NEXOBS;j++)
		{
			obs.data[i+usChOffSet].SNR[j]	= 0;
			obs.data[i+usChOffSet].LLI[j]	= 1;
			obs.data[i+usChOffSet].code[j]	= 0;
			obs.data[i+usChOffSet].L[j]		= 0;
			obs.data[i+usChOffSet].P[j]		= 0;
			obs.data[i+usChOffSet].D[j]		= 0;
		}
		
		obs.data[i+usChOffSet].time			= gps_time;
		if(usSvID>32)
		{
			obs.data[i+usChOffSet].sat			= usSvID-32+NSATGPS+NSATGLO+NSATGAL+NSATQZS;;//-32+NSATGPS+NSATGLO+NSATGAL+NSATQZS;
		}
		else 
		{
			obs.data[i+usChOffSet].sat			= usSvID;
		}
		// rcv类型，rover:1,base:2
		obs.data[i+usChOffSet].rcv			= 2;

		for(j=0;j<NFREQ+NEXOBS;j++)
		{			
			obs.data[i+usChOffSet].SNR[j]	= ptDiffObs->ucSnr[i][j];
			obs.data[i+usChOffSet].LLI[j]	= ptDiffObs->ucLostFlag[i][j];
			obs.data[i+usChOffSet].code[j]= 0;
			obs.data[i+usChOffSet].L[j]		= ptDiffObs->dCarrierPhase[i][j];
			obs.data[i+usChOffSet].P[j]		= ptDiffObs->dPseudoRange[i][j];
			obs.data[i+usChOffSet].D[j]		= ptDiffObs->fDoppler[i][j];
		}		
	}
	obs.n +=ptDiffObs->usNsats;
}



// RTK解算
/* update navigation data ----------------------------------------------------*/
//#pragma CODE_SECTION(updatenav,"sect_ECODE_II");
static void updatenav(nav_t *nav)
{
    int i,j;
    for (i=0;i<MAXSAT;i++) 
	{
	    for (j=0;j<NFREQ;j++) 
	    {
	        nav->lam[i][j]=satwavelen(i+1,j,nav);
	    }
	}
} 

//////////////////////////////////////////////////////////////////////////
/////////////////////////选择用于RTK解算的最优仰角////////////////////////
////////////////////////////////////////////////////////////////////////// 
unsigned short usGoodElevSatNum=0,usBetterElevSatNum=0,usBestElevSatNum=0;
double dAge;
unsigned short i,usSv,usInValidSat = 0;
unsigned short GetBestElev(TDIFFOBSINFOSTRUCT *ptDiffObs )
{
	
	
	
	usGoodElevSatNum			= 0;
	usBetterElevSatNum		= 0;
	usBestElevSatNum			= 0;
	usInValidSat 					= 0;
	
	for(i=0;i<MAXSAT;i++)
	{
		for(usSv=0;usSv<ptDiffObs->usNsats;usSv++)
		{
			if(ptDiffObs->usSvID[usSv] == i+1)
			{
				if(ptDiffObs->ucLostFlag[usSv][0]!=0)
				{
					usInValidSat++;
				}
			}
		}
		
		if(nav.eph[i].week == 0)
		{
			g_tSatElev.dElev[i] 					= 0.002;
			g_dElev[i]										= 0.002;
			g_tSatElev.uiCalcElevCount[i]	= 0;
			continue;
		}
		
		dAge 		= (float)(timediff(nav.eph[i].toe,obs.data[0].time));	
		
		if( fabs(dAge)>2*3600 )
		{
			g_tSatElev.dElev[i] 						= 0.002;
			g_dElev[i]											= 0.002;
			g_tSatElev.uiCalcElevCount[i]		= 0;
			continue;
		}		
		
		
		
		if(g_tSatElev.uiCalcElevCount[i]%60==3)
		{
			g_tSatElev.dElev[i] = g_dElev[i];
		}
		
		
		
		// 计算仰角大于10度的卫星数
		if( g_tSatElev.dElev[i] > 10*D2R )
		{
			usGoodElevSatNum++;
		}
		// 计算仰角大于12度的卫星数
		if( g_tSatElev.dElev[i] > 12*D2R )
		{
			usBetterElevSatNum++;
		}
		// 计算仰角大于15度的卫星数
		if( g_tSatElev.dElev[i] > 15*D2R )
		{
			usBestElevSatNum++;
		}
		
		g_tSatElev.uiCalcElevCount[i]++;
	}
	if(usBestElevSatNum>=15+usInValidSat)
	{
		return 1;
	}	
	else if(usBetterElevSatNum>=15+usInValidSat)
	{
		return 2;
	}
	else
	{
		return 3;
	}		
}

//////////////////////////////////////////////////////////////////////////
////////////////////////////////开始RTK解算///////////////////////////////
////////////////////////////////////////////////////////////////////////// 
//#pragma CODE_SECTION(CalcBaseLineWrok,"sect_ECODE_I"); 
void CalcBaseLineWrok( void )
{
	double dXYZ[3];
	unsigned short usElevSetFlag = 0;



	if(g_usRawRoverFlag == 0)
		return;
	
	//g_usRawRoverFlag = 0;
	
	//g_tRtk.opt.rb[0]	= -2154391.864;
	//g_tRtk.opt.rb[1]	= 4387771.004;
	//g_tRtk.opt.rb[2]	= 4083499.794;
	
	if (g_tBasePose.uiUserSetFlag == 1)
	{
		g_tRtk.opt.rb[0] = g_tReadPosFromBase.dUserSetX;
		g_tRtk.opt.rb[1] = g_tReadPosFromBase.dUserSetY;
		g_tRtk.opt.rb[2] = g_tReadPosFromBase.dUserSetZ;
	}
	/*else if(g_tBasePose.uiPosFilterFlag == 1)
	{
		g_tRtk.opt.rb[0] = g_tBasePose.dFilterX;
		g_tRtk.opt.rb[1] = g_tBasePose.dFilterY;
		g_tRtk.opt.rb[2] = g_tBasePose.dFilterZ;
	}*/
		
	
	memcpy(&nav.eph,&g_tRawRover.nav.eph,sizeof(nav.eph));
	nav.n=MAXSAT*1;
	nav.nmax=0;
	//			 nav.ng=NSATGLO*1;
	//			 nav.ngmax=0;

	// 卫星波长初始化
	updatenav(&nav);
	
	if (g_usRtkRatio<2.0)
	{
		memset(g_usCommonSat,0,sizeof(g_usCommonSat));
	}
	
	
	FormDiffObs( &g_tRawRover,&g_tDiffObsInfo );	

	FormRtkRoverObs( &g_tDiffObsInfo );
	
	/*usElevSetFlag = GetBestElev( &g_tDiffObsInfo );
	
	// 10已经是极限,双频一般都是用15的,10都不用
	// 信噪比,建议不屏蔽,
	// 最少多少颗星可以进入fix
	// 9颗都发生过(仰角30),但是有时候15颗都不行,关键是信号质量
	// 最好用大天线,四螺旋
	// 华信四螺旋天线,基本都在40以上,44~45,没有到47
	// ulbox的北斗做的不好
	if( usElevSetFlag == 1)
	{
		g_tRtk.opt.elmin = 15*D2R;
	}
	else if( usElevSetFlag == 2)
	{
		g_tRtk.opt.elmin = 12*D2R;
	}
	else 
	{
		g_tRtk.opt.elmin = 10*D2R;
	}*/

	
	// 基准站小于12颗星,就不更新了
	if(1==g_usRawBaseFlag)
	{	
		FormDiffObs( &g_tRawBase,&g_tRevBaseInfo );
		g_usRawBaseFlag = 0;
	}
	FormRtkBaseObs( &g_tRevBaseInfo );

					
	// 开始RTK差分定位
	rtkpos(&g_tRtk,obs.data,obs.n,&nav);

	g_usRtkRatio = g_tRtk.sol.ratio;
		
}    








