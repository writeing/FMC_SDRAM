#ifndef RTK_H
#define RTK_H

#include "global.h"

extern void RtkParaInit( void );
extern void FormRtkRoverObs( TDIFFOBSINFOSTRUCT *ptDiffObs  );
extern void FormRtkBaseObs( TDIFFOBSINFOSTRUCT *ptDiffObs );
extern void FormDiffObs( raw_t *ptRaw,TDIFFOBSINFOSTRUCT *ptDiffObs );
extern void CalcBaseLineWrok( void );

#endif 
