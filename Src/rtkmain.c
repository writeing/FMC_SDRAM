//#include <std.h>
#include <stdio.h>
//#include <c6x.h>
#include "global.h"

//#include "ProcessInt.h"

#include "rtkpnt.h"
#include "rtklib.h"
#include "rtk.h" 
//#include "c6747cfg.h"





extern int RtkMain( void )
{		

	// 初始化RTK参数
	InitGlobal();
	RtkParaInit();

	rtkinit(&g_tRtk,&prcopt_gps_doul);


	return 0;
}











