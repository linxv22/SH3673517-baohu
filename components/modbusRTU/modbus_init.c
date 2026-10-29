#include "esp_log.h"            // for log_write
#include "sdkconfig.h"
#include "mbcontroller.h"       // for mbcontroller defines and api
#include "common.h"
#include "soc/uart_reg.h"

static const char *TAG = "MODBUS_RTU";

#define MB_SLAVE_ADDR 1 
#define MB_PORT_NUM UART_NUM_2
#define CONFIG_MB_UART_TXD 17
#define CONFIG_MB_UART_RXD 18

#define MB_REG_HOLDING_START_AREA0  (0)
#define MB_REG_HOLD_CNT 100

// uint16_t holding_reg_area[MB_REG_HOLD_CNT] = {0,1,2,3,4,5,6,7,8,9,10};   // storage area for holding registers

SYSINFOR  mb_bat_inf ; //modbus发送缓冲区

const SYSINFOR con_mb_bat_inf = 
{
    // ========== 第4组：运行数据 ==========
    .ucCellNum    = 16,                    // 电芯串数：示例16串，按实际修改
    .ssVCell      = {[0 ... 19] = 3300},   // 20节电芯电压，默认3300mV(3.3V)
    .ssVCellMin   = 2900,                  // 最小电芯电压：默认3300mV
    .ssVCellMax   = 3400,                  // 最大电芯电压：默认3300mV
    .uiVoltage1   = 52800,                 // 电芯总压(和)：16串*3300mV
    .uiVoltage2   = 52800,                 // BAT+总压：默认与电芯总压一致
    .siCADCCurr   = 10000,                     // CADC平均电流：默认0A
    .siVADCCurr   = 20000,                     // VADC平均电流：默认0A
    .usTemp       = {250, 250, 250, 250},  // 4路外部温度：默认25.0℃
    .usTempMin    = 200,                   // 最小外部温度：25.0℃
    .usTempMax    = 260,                   // 最大外部温度：25.0℃
    .usTempI      = 240,                   // 芯片内部温度：25.0℃
    .uiVCHGD      = 10,                     // C+充电口电压：默认0
    .uiCTOChannel = 9,                     // 断线通道位：无断线
    .uiBALChannel = 8,                     // 均衡通道位：无均衡
    .usSoftVersion = 0x0001,               // 软件版本：示例 V1.0.0
    .usManufactureAccess = 1               // 厂商预留：默认0
};

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
        .ser_opts.port = MB_PORT_NUM,
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
    reg_area.address = (void*)&mb_bat_inf;     // Set pointer to storage instance
    reg_area.size = (sizeof(mb_bat_inf));    // Set the size of register storage area in bytes!
    reg_area.access = MB_ACCESS_RW;                     // Set the access rights for the area
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(mbc_slave_handle, reg_area));

    // Set UART pin numbers
    ESP_ERROR_CHECK(uart_set_pin(MB_PORT_NUM, CONFIG_MB_UART_TXD,
                                 CONFIG_MB_UART_RXD, UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE));

    // Set UART driver mode to UART_MODE_UART
    ESP_ERROR_CHECK(uart_set_mode(MB_PORT_NUM, UART_MODE_UART));

    // 自动收发方向的 485 收发器：方向由外部电路控制，无需 RTS 管脚。
    // 传输过程中置 RXBY_TX_EN=1、TX_RX_EN=0 以抑制回波(空字节)，任意 RS485 UART 模式下均有效。
    REG_SET_BIT(UART_RS485_CONF_REG(MB_PORT_NUM),
                UART_RS485RXBY_TX_EN);  // 置 1
    REG_CLR_BIT(UART_RS485_CONF_REG(MB_PORT_NUM),
                UART_RS485TX_RX_EN);    // 清 0


    // Starts of modbus controller and stack
    err = mbc_slave_start(mbc_slave_handle);

    mb_bat_inf = con_mb_bat_inf;
    return true;
}

/*************************************************************************************************
* 函数名: up_batinf
* 参  数: temp - 指向待更新的系统信息结构体指针，输入参数，内容不可修改
* 返回值: 无
* 描  述: 更新 Modbus 从机共享的电池系统信息数据。通过互斥锁保护全局变量 mb_bat_inf 的写入操作，
*         避免多任务并发访问引发的数据竞争与数据撕裂问题。
*************************************************************************************************/
void up_batinf(const SYSINFOR  *temp)
{
    // 入参合法性校验
    if (temp == NULL) {
        return; // 可根据业务增加错误日志/断言
    }
    mbc_slave_lock(mbc_slave_handle); // ignore the returned error if the object is not actual
    mb_bat_inf = *temp;// the data is part of initialized register area accessed by slave
    mbc_slave_unlock(mbc_slave_handle);
}
