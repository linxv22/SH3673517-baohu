#ifndef __SPIAPP_H
#define __SPIAPP_H

#include "common.h"

#define _USE_32BIT_MCU_								// 使用32位MCU

#define AFE_WRITE_CMD           0x01				// SH3673520写命令
#define AFE_READ_CMD            0x02				// SH3673520读命令
#define AFE_RESET_CMD           0x0B				// SH3673520软件复位命令

#define TRY_TIMES     			5

// #define SPI_CS_Pin				GPIO_PIN_4
// #define SPI_CS_GPIO_Port		GPIOA

// #define	GPIO_SPI_CSEN()         HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET)
// #define	GPIO_SPI_CSDis()        HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET)


extern BOOL bAfeSPIRWErrFlg;

extern BOOL SH_AFE_SPI_Init(void);

extern BOOL SH_AFE_SPI_SoftReset(void);
extern BOOL SH_AFE_SPI_Write(U8 RegAddr, U8 *WrData);
extern BOOL SH_AFE_SPI_Read(U8 RegAddr, U8 *RdBuf, U8 RdLen);

extern BOOL SH_AFE_SoftReset(void);
extern BOOL SH_AFE_Write(U8 RegAddr, U8 *WrData);
extern BOOL SH_AFE_Read(U8 RegAddr, U8 *RdBuf,U8 RdLen);

#endif
