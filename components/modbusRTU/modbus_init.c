#include "esp_log.h"            // for log_write
#include "sdkconfig.h"
#include "mbcontroller.h"       // for mbcontroller defines and api

static const char *TAG = "MODBUS_RTU";

static void *mbc_slave_handle = NULL;

/*************************************************************************************************
* 函数名: modbus_init
* 参  数: 无
* 返回值: true成功，false失败
* 描  述: 初始化modbus_rtu外设
*************************************************************************************************/
bool modbus_init(void)
{
   mb_communication_info_t comm_config = {
        .ser_opts.port = UART_NUM_2,
        .ser_opts.mode = MB_RTU,
        .ser_opts.baudrate = 115200,
        .ser_opts.parity = MB_PARITY_NONE,
        .ser_opts.uid = 1,
        .ser_opts.data_bits = UART_DATA_8_BITS,
        .ser_opts.stop_bits = UART_STOP_BITS_1
    };

    ESP_ERROR_CHECK(mbc_slave_create_serial(&comm_config, &mbc_slave_handle)); // Initialization of Modbus controller



    return true;
}

