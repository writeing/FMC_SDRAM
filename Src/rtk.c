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
///////////////////////////////RTK������ʼ��//////////////////////////////
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
/////////////////////////��ȡRTK��ʽ��BASE�۲���//////////////////////////
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
		if (usSvID>46)//��������һ������ʱ����
			//���˺�˼��,glonass������rtk,˵����������ļ���,����������Ҷ�����glonass�ź���rtk����
		//glonass��Ƶ�ʺܸ�,α�뾫�Ⱥܸ�
		//α��û��ƽ��,�����ܴ�,�ܵ�2~3m
		//rtk����˼��������ģ���������
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
		
		ptDiffObs->dCarrierPhase[usSatIndex][0] 	= ptRaw->obs.data[i].L[0]; // B1Ƶ����ز���
		ptDiffObs->dCarrierPhase[usSatIndex][1] 	= 0;
		ptDiffObs->dCarrierPhase[usSatIndex][2] 	= 0; // B3Ƶ����ز���
		
		ptDiffObs->dPseudoRange[usSatIndex][0] 		= ptRaw->obs.data[i].P[0]; // B1Ƶ����ز���
		ptDiffObs->dPseudoRange[usSatIndex][1] 		= 0; // B2Ƶ����ز���
		ptDiffObs->dPseudoRange[usSatIndex][2] 		= 0; // B3Ƶ����ز���
		
		ptDiffObs->ucLostFlag[usSatIndex][0]		= ptRaw->obs.data[i].LLI[0]; // 0:δʧ��
		ptDiffObs->ucLostFlag[usSatIndex][1]		= 1; // 0:ʧ��
		ptDiffObs->ucLostFlag[usSatIndex][2]		= 1; // 0:ʧ��
		
		ptDiffObs->fDoppler[usSatIndex][0]			= ptRaw->obs.data[i].D[0];
		ptDiffObs->fDoppler[usSatIndex][1]			= 0;
		ptDiffObs->fDoppler[usSatIndex][2]			= 0;
		
		ptDiffObs->ucSnr[usSatIndex][0]				= ptRaw->obs.data[i].SNR[0]/4; // B1Ƶ��������
		ptDiffObs->ucSnr[usSatIndex][1]				= 0; // B2Ƶ��������
		ptDiffObs->ucSnr[usSatIndex][2]				= 0; // B3Ƶ��������
		usSatIndex++;
		
	}
	ptDiffObs->usNsats	= usSatIndex;
} 

//////////////////////////////////////////////////////////////////////////
/////////////////////////��ȡRTK��ʽ��ROVER�۲���/////////////////////////
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
		// rcv���ͣ�rover:1,base:2
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
/////////////////////////��ȡRTK��ʽ��BASE�۲���//////////////////////////
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
		// rcv���ͣ�rover:1,base:2
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



// RTK����
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
/////////////////////////ѡ������RTK�������������////////////////////////
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
		
		
		
		// �������Ǵ���10�ȵ�������
		if( g_tSatElev.dElev[i] > 10*D2R )
		{
			usGoodElevSatNum++;
		}
		// �������Ǵ���12�ȵ�������
		if( g_tSatElev.dElev[i] > 12*D2R )
		{
			usBetterElevSatNum++;
		}
		// �������Ǵ���15�ȵ�������
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
////////////////////////////////��ʼRTK����///////////////////////////////
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

	// ���ǲ�����ʼ��
	updatenav(&nav);
	
	if (g_usRtkRatio<2.0)
	{
		memset(g_usCommonSat,0,sizeof(g_usCommonSat));
	}
	
	
	FormDiffObs( &g_tRawRover,&g_tDiffObsInfo );	

	FormRtkRoverObs( &g_tDiffObsInfo );
	
	/*usElevSetFlag = GetBestElev( &g_tDiffObsInfo );
	
	// 10�Ѿ��Ǽ���,˫Ƶһ�㶼����15��,10������
	// �����,���鲻����,
	// ���ٶ��ٿ��ǿ��Խ���fix
	// 9�Ŷ�������(����30),������ʱ��15�Ŷ�����,�ؼ����ź�����
	// ����ô�����,������
	// ��������������,��������40����,44~45,û�е�47
	// ulbox�ı������Ĳ���
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

	
	// ��׼վС��12����,�Ͳ�������
	if(1==g_usRawBaseFlag)
	{	
		FormDiffObs( &g_tRawBase,&g_tRevBaseInfo );
		g_usRawBaseFlag = 0;
	}
	FormRtkBaseObs( &g_tRevBaseInfo );

					
	// ��ʼRTK��ֶ�λ
	rtkpos(&g_tRtk,obs.data,obs.n,&nav);

	g_usRtkRatio = g_tRtk.sol.ratio;
		
}    








