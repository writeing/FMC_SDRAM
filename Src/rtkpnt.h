/*------------------------------------------------------------------------------
* rtkpnt.h  along 2012.07.13
* version 3.0  along 2013.05.12
*-----------------------------------------------------------------------------*/
#ifndef RTKPNT_H
#define RTKPNT_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAXNSAT      36//24 /* ������������Ŀ */

typedef struct {        /* ����ѡ�� */
	int navsys;         /* ����ϵͳ GPS: 1; ����: 2; ����: 3 */
	int pmode;          /* ��λģʽ (pmode=0:SPP; pmode=1:DGPS; pmode=2:RTK) */
	int nf;             /* Ƶ���� (nf=1:L1+B1; nf=2:L1/L2+B1/B2; nf=3:L1/L2/L5+B1/B2/B3)*/
    double elmin;       /* �߶Ƚ�ֹ�� ��λ: deg */
}PntOpt_t;

typedef struct {        		/* GPS/BDS������������ */
	unsigned char cSys;         /* ����ϵͳ GPS:1  BDS:2 */
	int iSatn;                  /* ���Ǳ��(GPS:1-32  BDS:1-14) */
	int iIode,iIodc;      		/* IODE,IODC */
	double dSva;            	/* URA (m) */
	int iSvh;            		/* ���ǽ�����־ */
	int iWeek;           		/* GPS�ܻ�BDS�� */
	double dToc,dTtr;     		/* Toc,T_trans */
	double dA,dE,dI0,dOMG0,dOmg;          /* ���ǹ������  dAΪ����������sqrtA��ƽ�� */
	double dM0,dDeln,dOMGd,dIdot;  	      /* ���ǹ������ */
	double dCrc,dCrs,dCuc,dCus,dCic,dCis; /* �㶯������ */
	double dToes;        			/* ������ */
	double dFit;         			/* ���ʱ�� (h) */
	double dF0,dF1,dF2;    			/* �����Ӳ���� (af0,af1,af2) */
	double dTgd[2];      			/* Ⱥ�ӳٲ��� */
}EphData_t;

typedef struct {         /* GPS/BDS�۲����� */
	unsigned char cSys;  /* ����ϵͳ(GPS:1  BDS:2)  */
	int iSatn;           /* ���Ǳ��(GPS:1-32  BDS:1-14) */
	double dL[3];        /* �ز���λ�۲�ֵ L1/B1 L2/B2 L5/B3 (cycle) */
	double dP[3];        /* α��۲�ֵ (m) */
	float  dD[3];        /* �����չ۲�ֵ (Hz) */
	unsigned char cSNR [3]; /* �ź�ǿ�� */
	unsigned char cLLI [3]; /* ���ջ�ʧ����־ */
}ObsData_t;

typedef struct {
	unsigned char iTsys;  /* ���ջ�ʱ��ϵͳ(GPST:0  BDT:1) */
	int iWeek;			  /* GPS�ܻ�BDS�� */
	double dSecond;		  /* GPS��BDS������ */

	int iValidSatNum;	  /* ��Ч������ (<MAXNSAT) */
	ObsData_t obs[MAXNSAT];     /* ���ǹ۲��������� */
}ObsArray_t;



/* rtkpnt��ʼ���ӿ�: ����������ʱ���� *****************************************/
//extern int InitRtkPnt();

/* rtkpntѡ�����ýӿ�: ����,������ѡ�����ñ仯ʱ���� ***************************
����ֵ: 1: ok  0: error
*******************************************************************************/
//extern int SetRtkPntOpt(PntOpt_t opt);

/* �����������ݴ��ͽӿ�: ���ջ����������ʱ���� ********************************
����ֵ: �����������Ǳ��: ok  <0: error
*******************************************************************************/
//extern int SendOneEphToRtkPnt(EphData_t eph);

/* ��Ԫ�۲����ݴ��ͺ�rtkpnt����ӿ�: ÿһ��Ԫ���� ******************************
rb[]: ��׼վ���� ecef-xyz
Refdata: ��׼վ�۲�����
Rovdata: ����վ�۲�����
result: rtkpnt��λ���
����ֵ: 1: ok  0: error
*******************************************************************************/
//extern int SendOneEpochToRtkPnt(double rb[3], ObsArray_t Refdata,
//								ObsArray_t Rovdata, RTK_RESULT *result);

#ifdef __cplusplus
}
#endif

#endif   /* RTKPNT_H */
