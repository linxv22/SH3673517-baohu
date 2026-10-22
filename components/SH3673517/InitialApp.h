#ifndef __INITIALAPP_H
#define __INITIALAPP_H

#include "common.h"

#define SOFT_SERVION		0x0012			// 软件版本号 V1.2

#define CELLNUM 0x10        //15串
#define TSNUM   2          //2路

extern void UserInitial(void);
extern void InitSysPara(void);
extern void SystemResetProcess(void);

extern void ResetInit(void);
#endif

