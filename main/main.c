#include <stdio.h>

#include "common.h"
#include "SPIApp.h"
#include "InitialApp.h"


void app_main(void)
{

    SH_AFE_SPI_Init();

    UserInitial();

}
