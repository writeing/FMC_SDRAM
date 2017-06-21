/*------------------------------------------------------------------------------
* rtkpnt.h  along 2012.07.13
* version 3.0  along 2013.05.12
*-----------------------------------------------------------------------------*/
#ifndef RTKPNT_H
#define RTKPNT_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAXNSAT      36//24 /* 最大跟踪卫星数目 */

typedef struct {        /* 处理选项 */
	int navsys;         /* 卫星系统 GPS: 1; 北斗: 2; 兼容: 3 */
	int pmode;          /* 定位模式 (pmode=0:SPP; pmode=1:DGPS; pmode=2:RTK) */
	int nf;             /* 频率数 (nf=1:L1+B1; nf=2:L1/L2+B1/B2; nf=3:L1/L2/L5+B1/B2/B3)*/
    double elmin;       /* 高度截止角 单位: deg */
}PntOpt_t;

typedef struct {        		/* GPS/BDS卫星星历数据 */
	unsigned char cSys;         /* 卫星系统 GPS:1  BDS:2 */
	int iSatn;                  /* 卫星编号(GPS:1-32  BDS:1-14) */
	int iIode,iIodc;      		/* IODE,IODC */
	double dSva;            	/* URA (m) */
	int iSvh;            		/* 卫星健康标志 */
	int iWeek;           		/* GPS周或BDS周 */
	double dToc,dTtr;     		/* Toc,T_trans */
	double dA,dE,dI0,dOMG0,dOmg;          /* 卫星轨道参数  dA为星历参数中sqrtA的平方 */
	double dM0,dDeln,dOMGd,dIdot;  	      /* 卫星轨道参数 */
	double dCrc,dCrs,dCuc,dCus,dCic,dCis; /* 摄动改正项 */
	double dToes;        			/* 周内秒 */
	double dFit;         			/* 拟合时间 (h) */
	double dF0,dF1,dF2;    			/* 卫星钟差参数 (af0,af1,af2) */
	double dTgd[2];      			/* 群延迟参数 */
}EphData_t;

typedef struct {         /* GPS/BDS观测数据 */
	unsigned char cSys;  /* 卫星系统(GPS:1  BDS:2)  */
	int iSatn;           /* 卫星编号(GPS:1-32  BDS:1-14) */
	double dL[3];        /* 载波相位观测值 L1/B1 L2/B2 L5/B3 (cycle) */
	double dP[3];        /* 伪距观测值 (m) */
	float  dD[3];        /* 多普勒观测值 (Hz) */
	unsigned char cSNR [3]; /* 信号强度 */
	unsigned char cLLI [3]; /* 接收机失锁标志 */
}ObsData_t;

typedef struct {
	unsigned char iTsys;  /* 接收机时间系统(GPST:0  BDT:1) */
	int iWeek;			  /* GPS周或BDS周 */
	double dSecond;		  /* GPS或BDS周内秒 */

	int iValidSatNum;	  /* 有效卫星数 (<MAXNSAT) */
	ObsData_t obs[MAXNSAT];     /* 卫星观测数据数组 */
}ObsArray_t;



/* rtkpnt初始化接口: 开机或重启时调用 *****************************************/
//extern int InitRtkPnt();

/* rtkpnt选项设置接口: 开机,重启和选项设置变化时调用 ***************************
返回值: 1: ok  0: error
*******************************************************************************/
//extern int SetRtkPntOpt(PntOpt_t opt);

/* 卫星星历数据传送接口: 接收机解调出星历时调用 ********************************
返回值: 输入星历卫星编号: ok  <0: error
*******************************************************************************/
//extern int SendOneEphToRtkPnt(EphData_t eph);

/* 历元观测数据传送和rtkpnt解算接口: 每一历元调用 ******************************
rb[]: 基准站坐标 ecef-xyz
Refdata: 基准站观测数据
Rovdata: 流动站观测数据
result: rtkpnt定位结果
返回值: 1: ok  0: error
*******************************************************************************/
//extern int SendOneEpochToRtkPnt(double rb[3], ObsArray_t Refdata,
//								ObsArray_t Rovdata, RTK_RESULT *result);

#ifdef __cplusplus
}
#endif

#endif   /* RTKPNT_H */
