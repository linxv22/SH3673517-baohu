#include "esp_log.h"            // for log_write
#include "sdkconfig.h"
#include "mbcontroller.h"       // for mbcontroller defines and api

static const char *TAG = "MODBUS_RTU";

#define MB_SLAVE_ADDR 1 
#define MB_PORT_NUM UART_NUM_2
#define CONFIG_MB_UART_TXD 17
#define CONFIG_MB_UART_RXD 18

#define MB_REG_HOLDING_START_AREA0  (0)
#define MB_REG_HOLD_CNT 100

uint16_t holding_reg_area[MB_REG_HOLD_CNT] = {0,1,2,3,4,5,6,7,8,9,10};   // storage area for holding registers

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
        .ser_opts.mode = MB_RTU,                // RTU communication mode
        .ser_opts.baudrate = 115200,
        .ser_opts.parity = MB_PARITY_NONE,
        .ser_opts.uid = MB_SLAVE_ADDR,          // Modbus slave UID - Unit Identifier (short address)
        .ser_opts.data_bits = UART_DATA_8_BITS,
        .ser_opts.stop_bits = UART_STOP_BITS_1
    };

    esp_err_t err = mbc_slave_create_serial(&comm_config, &mbc_slave_handle); // Initialization of Modbus controller

    if (mbc_slave_handle == NULL || err != ESP_OK) {
    ESP_LOGE(TAG, "mb controller initialization fail.");
    return false;
    }

    mb_register_area_descriptor_t reg_area;             // Modbus register area descriptor structure
   
    reg_area.type = MB_PARAM_HOLDING;                   // Set type of register area
    reg_area.start_offset = MB_REG_HOLDING_START_AREA0; // Offset of register area in Modbus protocol
    reg_area.address = (void*)&holding_reg_area[0];     // Set pointer to storage instance
    reg_area.size = (sizeof(holding_reg_area) << 1);    // Set the size of register storage area in bytes!
    reg_area.access = MB_ACCESS_RW;                     // Set the access rights for the area
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(mbc_slave_handle, reg_area));

    // Set UART pin numbers
    ESP_ERROR_CHECK(uart_set_pin(MB_PORT_NUM, CONFIG_MB_UART_TXD,
                                 CONFIG_MB_UART_RXD, UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE));

    // Set UART driver mode to UART_MODE_UART
    ESP_ERROR_CHECK(uart_set_mode(MB_PORT_NUM, UART_MODE_UART));

    // Starts of modbus controller and stack
    err = mbc_slave_start(mbc_slave_handle);

    return true;
}
