/********************************************************************************
Copyright (C), Sinowealth Electronic. Ltd.
Author: 	Sino
Version: 	V0.0
Date: 		2024/05/11
History:
	V0.0		2024/05/11		 Preliminary
********************************************************************************/
#include "InitialApp.h"
#include "AFE.h"

// <h>电池包配置(E2uiPackConfigMap)

// <q>预放电控制
#define _EPCM_PDSGMOS_EN			0		    // BIT0; 0：不支持预放电;	1：支持预放电
// <q>均衡功能
#define _EPCM_BAL_EN				1		    // BIT1; 0：不支持均衡功能;	1：支持均衡功能
// <q>断线功能
#define _EPCM_CTO_EN				0		    // BIT2; 0：不支持断线功能;	1：支持断线功能


#define	_E2_PACKCONFIGMAP           ((_EPCM_CTO_EN<<2)\
                                    |(_EPCM_BAL_EN<<1)|(_EPCM_PDSGMOS_EN))
// </h>
// <h>单节过欠压保护释放
// <o>单节过压保护释放电压(mV)<0-5000>
#define	_E2_OVR_VOL					4100    // 4100mV
// <o>单节欠压保护释放电压(mV)<0-5000>
#define	_E2_UVR_VOL					3000		// 3000mV1
// <o>单节过压保护释放延时(70mS)
#define	_E2_OVR_DELAY				14			// 0.98s
// <o>单节欠压保护释放延时(70mS)		// 0.98s
// </h>
#define	_E2_UVR_DELAY				14	

// <h>高低温保护保护恢复
// <o>充电高温恢复阈值(2731+10*x℃)<2331-3581>
// <i>由于配置向导无法处理负数，低温保护输入值=2731+10*x，其中x为设置温度，如需要设置的保护温度为-10℃，输入值=2731+10*(-10)=2631
#define	_E2_OTCR_TEMP				3181		// 45℃
// <o>充电低温恢复阈值(2731+10*x℃)<2331-3581>
// <i>由于配置向导无法处理负数，低温保护输入值=2731+10*x，其中x为设置温度，如需要设置的保护温度为-10℃，输入值=2731+10*(-10)=2631
#define	_E2_UTCR_TEMP				2781		// 5℃
// <o>放电高温恢复阈值(2731+10*x℃)<2331-3581>
// <i>由于配置向导无法处理负数，低温保护输入值=2731+10*x，其中x为设置温度，如需要设置的保护温度为-10℃，输入值=2731+10*(-10)=2631
#define	_E2_OTDR_TEMP				3281		// 55℃
// <o>放电低温恢复阈值(2731+10*x℃)<2331-3581>
// <i>由于配置向导无法处理负数，低温保护输入值=2731+10*x，其中x为设置温度，如需要设置的保护温度为-10℃，输入值=2731+10*(-10)=2631
#define	_E2_UTDR_TEMP				2681		// -5℃
// <o>温度保护释放延时(70mS)
#define	_E2_OUTR_DELAY				28			// 1.96s
// </h>

// <h>总压过欠压保护
// <o>总压欠压保护阈值(0.1V)<0-1000>
#define	_E2_PACKUV_VOL				400			// 40.0V
// <o>总压欠压恢复阈值(0.1V)<0-1000>
#define	_E2_PACKUVR_VOL				455			// 45.5V
// <o>总压欠压延时(70mS)
#define	_E2_PACKUV_DELAY			14			// 0.98s
// <o>总压欠压恢复延时(70mS)
#define	_E2_PACKUVR_DELAY			14			// 0.98s
// <o>总压过压保护阈值(0.1V)<0-1000>
#define	_E2_PACKOV_VOL				630			// 63.0V
// <o>总压过压恢复阈值(0.1V)<0-1000>
#define	_E2_PACKOVR_VOL				610			// 61.0V
// <o>总压过压延时(70mS)
#define	_E2_PACKOV_DELAY			14			// 0.98s
// <o>总压过压恢复延时(70mS)
#define	_E2_PACKOVR_DELAY			14			// 0.98s
//</h>

// <h>平衡配置
// <o>平衡开启电压(mV)<0-5000>
#define	_E2_BAL_VOL					4130		// 4130mV
// <o>平衡开启充电电流(mA)<0x0-0x7ff>
#define	_E2_BALANCE_CURRENT			100			// 100mA
// <o>平衡开启压差(mV)<0-5000>
#define	_E2_BAL_VOLDIFF				30			// 30mV
// <o>平衡开启延时(70mS)
#define	_E2_BAL_DELAY				29			// 2.03s
//</h>

// <o>充电器接入检测阈值(0.1V)<0-1000>
#define	_E2_CHGCHK_VOL				630			// 63.0V
// <o>充电器拔除检测阈值(0.1V)<0-1000>
#define	_E2_CHGRCHK_VOL				600			// 60.0V
// <o>负载/充电器检测延时(70mS)
#define	_E2_LOADCHGCHK_DELAY		14			// 0.98s
// <o>零电流允许偏差(mA)<0-65535>
#define _E2_DFILTERCUR              100			// 100mA
//</h>


// <o>AFE的SCONF1<0x0-0xff>
#define	_E2_AFE_SCONF1              0x00
// <o>AFE的SCONF2<0x0-0xff>
#define	_E2_AFE_SCONF2              0x10		// PUMP_EN = 1
// <o>AFE的SCONF3<0x0-0xff>
#define	_E2_AFE_SCONF3              0x40		// CGR_WK = 1
// <o>AFE的SCONF4<0x0-0xff>
#define	_E2_AFE_SCONF4              (0x60 | CELLNUM)
// <o>AFE的SCONF5<0x0-0xff>
#define	_E2_AFE_SCONF5              0x38		// MOS_EN = 1
// <o>AFE的SCONF6<0x0-0xff>
#define	_E2_AFE_SCONF6              0x3F
// <o>AFE的SCONF7<0x0-0xff>
#define	_E2_AFE_SCONF7              0x04
// <o>AFE的OWV_ALARMH<0x0-0xff>
#define	_E2_AFE_OWV_ALARMH          0x57
// <o>AFE的ALARML<0x0-0xff>
#define	_E2_AFE_ALARML              0xFF
// <o>  AFE的过充电保护延时(OVT)
// <0x00=> 140ms
// <0x10=> 280ms
// <0x20=> 490ms
// <0x30=> 0.98s
// <0x40=> 2.03s
// <0x50=> 3.01s
// <0x60=> 4.97s
// <0x70=> 10.01s
#define _E2_AFE_OVT					0x30
// <o> AFE的过充电保护电压(OV),5mV/Step<0-4500>
#define _E2_AFE_OV					4175
#define	_E2_AFE_OVT_OVH             (uint8_t)(_E2_AFE_OVT | ((_E2_AFE_OV/5)>>8))
#define	_E2_AFE_OVL                 (uint8_t)(_E2_AFE_OV/5)

// <o> AFE的过放电保护延时(OVT)
// <0x00=> 490ms
// <0x10=> 770ms
// <0x20=> 0.98s
// <0x30=> 1.47s
// <0x40=> 2.03s
// <0x50=> 3.01s
// <0x60=> 4.97s
// <0x70=> 10.01s
#define _E2_AFE_UVT					0x40
// <o> AFE的过放电保护电压(UV),5mV/Step<0-4500>
#define _E2_AFE_UV					2700
#define	_E2_AFE_UVT_UVH             (uint8_t)(_E2_AFE_UVT | ((_E2_AFE_UV/5)>>8))
#define	_E2_AFE_UVL                 (uint8_t)(_E2_AFE_UV/5)

// <o> AFE的放电过流1保护延时(OCD1T)
// <0x00=> 140ms
// <0x10=> 280ms
// <0x20=> 490ms
// <0x30=> 0.98s
// <0x40=> 2.03s
// <0x50=> 3.01s
// <0x60=> 4.97s
// <0x70=> 10.01s
#define _E2_AFE_OCD1T				0x30
// <o> AFE的放电过流1保护阈值(OCD1V)
// <0x00=> 5mV
// <0x01=> 10mV
// <0x02=> 15mV
// <0x03=> 20mV
// <0x04=> 25mV
// <0x05=> 30mV
// <0x06=> 35mV
// <0x07=> 40mV
// <0x08=> 45mV
// <0x09=> 50mV
// <0x0A=> 55mV
// <0x0B=> 60mV
// <0x0C=> 65mV
// <0x0D=> 70mV
// <0x0E=> 75mV
// <0x0F=> 80mV
#define _E2_AFE_OCD1V				0x0A
#define	_E2_AFE_OCD1V_OCD1T			(uint8_t)(_E2_AFE_OCD1T | _E2_AFE_OCD1V)

// <o> AFE的放电过流2保护延时(OCD2T)
// <0x00=> 25ms
// <0x10=> 50ms
// <0x20=> 75ms
// <0x30=> 100ms
// <0x40=> 125ms
// <0x50=> 150ms
// <0x60=> 175ms
// <0x70=> 200ms
// <0x80=> 225ms
// <0x90=> 250ms
// <0xA0=> 275ms
// <0xB0=> 300ms
// <0xC0=> 325ms
// <0xD0=> 350ms
// <0xE0=> 375ms
// <0xF0=> 400ms
#define _E2_AFE_OCD2T				0xF0
// <o> AFE的放电过流2保护阈值(OCD2V)
// <0x00=> 10mV
// <0x01=> 20mV
// <0x02=> 30mV
// <0x03=> 40mV
// <0x04=> 50mV
// <0x05=> 60mV
// <0x06=> 70mV
// <0x07=> 80mV
// <0x08=> 90mV
// <0x09=> 100mV
// <0x0A=> 110mV
// <0x0B=> 120mV
// <0x0C=> 130mV
// <0x0D=> 140mV
// <0x0E=> 150mV
// <0x0F=> 160mV
#define _E2_AFE_OCD2V				0x0B
#define	_E2_AFE_OCD2V_OCD2T			(uint8_t)(_E2_AFE_OCD2T | _E2_AFE_OCD2V)

// <o> AFE的短路保护延时(SCT)
// <0x00=> 0us
// <0x01=> 32us
// <0x02=> 64us
// <0x03=> 96us
// <0x04=> 128us
// <0x05=> 192us
// <0x06=> 224us
// <0x07=> 256us
// <0x08=> 288us
// <0x09=> 320us
// <0x0A=> 384us
// <0x0B=> 448us
// <0x0C=> 480us
// <0x0D=> 512us
// <0x0E=> 544us
// <0x0F=> 576us
#define _E2_AFE_SCT					0x07
// <o> AFE的短路保护阈值(SCV)
// <0x00=> 2倍的放电过流2保护阈值
// <0x10=> 3倍的放电过流2保护阈值
// <0x20=> 4倍的放电过流2保护阈值
// <0x30=> 6倍的放电过流2保护阈值
#define _E2_AFE_SCV					0x00
#define	_E2_AFE_SCV_SCT				(uint8_t)(_E2_AFE_SCT | _E2_AFE_SCV)

// <o> AFE的充电过流保护延时(OCCT)
// <0x00=> 140ms
// <0x20=> 280ms
// <0x40=> 490ms
// <0x60=> 0.98s
// <0x80=> 2.03s
// <0xA0=> 3.01s
// <0xC0=> 4.97s
// <0xE0=> 10.01s
#define _E2_AFE_OCCT				0x60
// <o> AFE的充电过流保护阈值(OCCV)
// 寄存器值*1.375mV+1.375mV
#define _E2_AFE_OCCV					0x09
#define	_E2_AFE_OCCV_OCCT				(uint8_t)(_E2_AFE_OCCT | _E2_AFE_OCCV)

// <o>AFE的OTC(2731+10*x℃)<2331-3581> 充电
// <i>由于配置向导无法处理负数，高温保护输入值=2731+10*x，其中x为设置温度，如需要设置的保护温度为-10℃，输入值=2731+10*(-10)=2631
#define	_E2_AFE_OTC_TEMP			3231		// 50℃
// <o>AFE的OTD(2731+10*x℃)<2331-3581> 放电
// <i>由于配置向导无法处理负数，高温保护输入值=2731+10*x，其中x为设置温度，如需要设置的保护温度为-10℃，输入值=2731+10*(-10)=2631
#define	_E2_AFE_OTD_TEMP			3331		// 60℃
// <o>AFE的UTC(2731+10*x℃)<2331-3581> 充电
// <i>由于配置向导无法处理负数，低温保护输入值=2731+10*x，其中x为设置温度，如需要设置的保护温度为-10℃，输入值=2731+10*(-10)=2631
#define	_E2_AFE_UTC_TEMP			2731		// 0℃
// <o>AFE的UTD(2731+10*x℃)<2331-3581>
// <i>由于配置向导无法处理负数，低温保护输入值=2731+10*x，其中x为设置温度，如需要设置的保护温度为-10℃，输入值=2731+10*(-10)=2631
#define	_E2_AFE_UTD_TEMP			2631		// -10℃

//</h>


// <h>校准参数(SubClassID=0x0B,langth=15)
// <o>AFE总电压增益1(uint16_t)<0x0-0xffff>
#define	_E2_VPACK_GAIN1				32000		// uint16_t E2usVPackGain1
// <o>AFE总电压增益2(uint16_t)<0x0-0xffff>
#define	_E2_VPACK_GAIN2				12800		// uint16_t E2usVPackGain2
// <o>VADC电流增益(S16)<0x0-0x7fff>
#define	_E2_CURRENT_GAIN1			2913		// S16 E2ssCurrGain1
// <o>VADC零电流偏移(S16)<0x0-0x7fff>
#define	_E2_CURRENT_OFFSET1			-11			// S16 E2ssCurrOffset1
// <o>CADC电流增益(S16)<0x0-0x7fff>
#define	_E2_CURRENT_GAIN2			2913		// S16 E2ssCurrGain2
// <o>VADC零电流偏移(S16)<0x0-0x7fff>
#define	_E2_CURRENT_OFFSET2			5			// S16 E2ssCurrOffset2
// <o>外部温度1偏移量(S16)<0x0-0x7fff>
#define	_E2_TS1_OFFSET              0			// S16 E2ssTempe1Offset
// <o>外部温度2偏移量(S16)<0x0-0x7fff>
#define _E2_TS2_OFFSET              0			// S16 E2ssTempe2Offset
// <o>环境温度偏移量(S16)<0x0-0x7fff>
#define	_E2_TS3_OFFSET              0			// S16 E2ssTempe4Offset
// <o>MOS温度偏移量(S16)<0x0-0x7fff>
#define _E2_TS4_OFFSET				0			// S16 E2ssTempe5Offset

const PARAMETER constParams = 
{
	.E2usPackConfigMap		= _E2_PACKCONFIGMAP,
	
	.E2usPackUVVol			= _E2_PACKUV_VOL,  
	.E2usPackUVRVol			= _E2_PACKUVR_VOL,  
	.E2ucPackUVDelay		= _E2_PACKUV_DELAY,
	.E2ucPackUVRDelay		= _E2_PACKUVR_DELAY,
	
	.E2usPackOVVol			= _E2_PACKOV_VOL,  
	.E2usPackOVRVol			= _E2_PACKOVR_VOL,  
	.E2ucPackOVDelay		= _E2_PACKOV_DELAY, 
	.E2ucPackOVRDelay		= _E2_PACKOVR_DELAY,
	
	.E2usOVRVol				= _E2_OVR_VOL,
	.E2usUVRVol				= _E2_UVR_VOL,
	.E2usUTCRTemp			= _E2_UTCR_TEMP,
	.E2usOTCRTemp			= _E2_OTCR_TEMP,
	.E2usUTDRTemp			= _E2_UTDR_TEMP,
	.E2usOTDRTemp			= _E2_OTDR_TEMP,
	
	.E2ucOVRDelay			= _E2_OVR_DELAY,
	.E2ucUVRDelay			= _E2_UVR_DELAY,
	.E2ucOUTRDelay			= _E2_OUTR_DELAY,
	
	.E2ucLoadChgChkDelay	= _E2_LOADCHGCHK_DELAY,
	.E2usChgChkVol			= _E2_CHGCHK_VOL,
	.E2usChgRChkVol			= _E2_CHGRCHK_VOL,
	
	.E2usBalanceVol			= _E2_BAL_VOL,
	.E2usBalanceVolDiff		= _E2_BAL_VOLDIFF,
	.E2ssBalanceCur			= _E2_BALANCE_CURRENT,
	.E2ucBalanceDelay		= _E2_BAL_DELAY,
	.E2ssDfilterCur			= _E2_DFILTERCUR,
	
	
	// AFE参数区开始 
	.E2usAFEOTCTemp			= _E2_AFE_OTC_TEMP,
	.E2usAFEOTDTemp			= _E2_AFE_OTD_TEMP,
	.E2usAFEUTCTemp			= _E2_AFE_UTC_TEMP,
	.E2usAFEUTDTemp			= _E2_AFE_UTD_TEMP,
	
	.E2ucAFESCONF1			= _E2_AFE_SCONF1,
	.E2ucAFESCONF2			= _E2_AFE_SCONF2,
	.E2ucAFESCONF3			= _E2_AFE_SCONF3,
	.E2ucAFESCONF4			= _E2_AFE_SCONF4,
	.E2ucAFESCONF5			= _E2_AFE_SCONF5, 
	.E2ucAFESCONF6			= _E2_AFE_SCONF6,  
	.E2ucAFESCONF7			= _E2_AFE_SCONF7,   
	.E2ucAFEOWV_ALARMH		= _E2_AFE_OWV_ALARMH,
	.E2ucAFEALARML			= _E2_AFE_ALARML,
	.E2ucAFEOVT_OVH			= _E2_AFE_OVT_OVH,
	.E2ucAFEOVL				= _E2_AFE_OVL,
	.E2ucAFEUVT_UVH			= _E2_AFE_UVT_UVH,
	.E2ucAFEUVL				= _E2_AFE_UVL,
	.E2ucAFEOCD1V_OCD1T		= _E2_AFE_OCD1V_OCD1T,
	.E2ucAFEOCD2V_OCD2T		= _E2_AFE_OCD2V_OCD2T,
	.E2ucAFESCV_SCT			= _E2_AFE_SCV_SCT,
	.E2ucAFEOCCV_OCCT		= _E2_AFE_OCCV_OCCT,

	
	//校准参数区开始 SubClassID=0xB			langth=20
	.E2usVPackGain1			= _E2_VPACK_GAIN1,
	.E2usVPackGain2			= _E2_VPACK_GAIN2,
	.E2ssCurrGain1			= _E2_CURRENT_GAIN1,
	.E2ssCurrOffset1		= _E2_CURRENT_OFFSET1,
	.E2ssCurrGain2			= _E2_CURRENT_GAIN2,
	.E2ssCurrOffset2		= _E2_CURRENT_OFFSET2,
	.E2ssTSnOffset[0]		= _E2_TS1_OFFSET,
	.E2ssTSnOffset[1]		= _E2_TS2_OFFSET,
	.E2ssTSnOffset[2]		= _E2_TS3_OFFSET,
	.E2ssTSnOffset[3]		= _E2_TS4_OFFSET,
};


BOOL bMcuFlashErr;

/*************************************************************************************************
* 函数名: InitVar
* 参  数: 无
* 返回值: 无
* 描  述: 初始化用户自定义变量
*************************************************************************************************/
void InitVar(void)
{
	if(Info.bHighSide)						// 高侧方案时，开启Charge Pump(PUMP_EN = 1)
	{
		parameter.E2ucAFESCONF2 = parameter.E2ucAFESCONF2|0x10;
		
		if(parameter.bPDSGMOS_EN)							// 开启预充电时，配置PDSGMOS=1，预放电功能由SH3673520控制。
		{
			parameter.E2ucAFESCONF2 = parameter.E2ucAFESCONF2|0x04;
		}
	}
	
	Info.siCADCCurr = 0; 									// After the program is reset, the current default is "0"
	Info.usPackConfig = parameter.E2usPackConfigMap;
	
	Info.uiCTOChannel = 0;
	Info.uiBALChannel = 0;
	Info.usSoftVersion = SOFT_SERVION;
	
	Info.ucCellNum = (parameter.E2ucAFESCONF4 & 0x1F);		// 根据配置初始化电芯串数和Offset
	if ((Info.ucCellNum >= 17) || (Info.ucCellNum < 4))
	{
		Info.ucCellNum = 17;
	}
}

/*************************************************************************************************
* 函数名: InitSysPara
* 参  数: 无
* 返回值: 无
* 描  述: 初始化系统参数，从Flash区读取对应参数
*************************************************************************************************/
void InitSysPara(void)
{

	parameter = constParams;

}

/*************************************************************************************************
* 函数名: UserInitial
* 参  数: 无
* 返回值: 无
* 描  述: 系统初始化
*************************************************************************************************/
void UserInitial(void)
{
	Info.bHighSide = 0;//低边
	// if(HAL_GPIO_ReadPin(HL_Side_GPIO_Port,HL_Side_Pin))		// PB14管脚悬空：高侧方案。PB14管脚下拉到GND：低侧方案。
	// {
	// 	Info.bHighSide = 1;
	// }
	
	InitSysPara();					// 系统参数初始化
	
	InitVar();						// 初始化变量
	
	if(!SH_AFE_RegisterInit())		// 初始化AFE主控
	{
		Info.bAFE_ERR = 1;			// 第一次如果初始化失败则置位该标志，中途是连续判断5S置位该标志
	}
	SH_AFE_ClearAllProtectFlag();	// 清零AFE状态寄存器
}

/*******************************************************************************
* 函数名: ResetInit
* 参  数: 无
* 返回值: 无
* 描  述: MCU复位
*******************************************************************************/
void ResetInit(void)
{
	// IWDG_Refresh();					// 喂狗
	// __disable_irq();				// 关闭总中断
	// SYS_EXCOM_ABORT();				// 系统对外通讯操作全部终止
	// NVIC_SystemReset();				// MCU复位
}

/*************************************************************************************************
* 函数名: SystemResetProcess
* 参  数: 无
* 返回值: 无
* 描  述: 系统复位处理，当接收到上位机复位命令时复位MCU
*************************************************************************************************/
void SystemResetProcess(void)
{
	// //收到上位机复位命令并回复完Ack
	// if((ucResetFlg == 0x12) && bUart2SndAckFlg)
	// {
	// 	if(!bWriteFlashFlg)				// 等待Flash写入完成
	// 	{
	// 		ResetInit();				// Reset MCU
	// 	}
	// }
}
