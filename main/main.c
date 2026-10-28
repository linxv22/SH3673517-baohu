#include <stdio.h>

#include "common.h"
#include "SPIApp.h"
#include "InitialApp.h"
#include "AFE.h"
#include "Calculate.h"
#include "modbus_init.h"
#include "sd_init.h"

void app_main(void)
{

    // SH_AFE_SPI_Init();
    // UserInitial();
    modbus_init();
    sd_init();
    while (1)
    {
        // SH_AFE_ReadRegister(Info.ucCellNum, TSNUM); // 读取AFE中存储电池状态信息的寄存器
        // SH_AFE_GetBattInfo(Info.ucCellNum, TSNUM, Info.bHighSide); // 获取电池的电压、温度、电流和状态
        
        vTaskDelay(70 / portTICK_PERIOD_MS);
    }
    


}
