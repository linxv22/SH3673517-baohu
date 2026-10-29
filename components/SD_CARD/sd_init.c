#include "sd_init.h"

#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"


#define ESP0_SD_CLK 15
#define ESP0_SD_CMD 6
#define ESP0_SD_DAT0 7

#define TAG "SD"
#define MOUNT_POINT "/sdcard"
#define BMS_POINT MOUNT_POINT"/BMS"

static SYSINFOR sd_SYSINFOR ={

    .ucCellNum = 16,

};

static void BMS_save(void *arg);

void sd_init(void)
{

    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {

        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.

    ESP_LOGI(TAG, "Using SDMMC peripheral");

    // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
    // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 40MHz for SDMMC)
    // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;
    // On chips where the GPIOs used for SD card can be configured, set them in
    // the slot_config structure:
    slot_config.clk = ESP0_SD_CLK;
    slot_config.cmd = ESP0_SD_CMD;
    slot_config.d0 = ESP0_SD_DAT0;
    // Enable internal pullups on enabled pins. The internal pullups
    // are insufficient however, please make sure 10k external pullups are
    // connected on the bus. This is for debug / example purpose only.
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    ESP_LOGI(TAG, "Mounting filesystem");
        if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    xTaskCreate(BMS_save, "bms_test", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "循环写任务启动");

}

static int build_row(char *line, const SYSINFOR *s, uint32_t num)
{
    int n = 0;
    static int t = 1;
    n += sprintf(line + n, "%lu,", (unsigned long)num);                 /* 1  次数 */
    n += sprintf(line + n, "0x%04X,", s->usPackStatus);                 /* 2  电池包状态 */
    n += sprintf(line + n, "0x%04X,", s->usBatStatus);                  /* 3  保护状态 */
    n += sprintf(line + n, "0x%04X,", s->usPackConfig);                 /* 4  电池包设置 */
    n += sprintf(line + n, "%u,",     s->ucCellNum);                    /* 5  电芯串数 */

    for (int i = 0; i < 20; i++) {                                      /* 6-25 电芯电压 */
        if (i < (int)s->ucCellNum)
            n += sprintf(line + n, "%d", s->ssVCell[i]+t);
        line[n++] = ',';
    }

    n += sprintf(line + n, "%d,",  s->ssVCellMin);                      /* 26 */
    n += sprintf(line + n, "%d,",  s->ssVCellMax);                      /* 27 */
    n += sprintf(line + n, "%lu,", (unsigned long)s->uiVoltage1);       /* 28 电芯总压 */
    n += sprintf(line + n, "%lu,", (unsigned long)s->uiVoltage2);       /* 29 BAT+总压 */
    n += sprintf(line + n, "%ld,", (long)s->siCADCCurr);                /* 30 */
    n += sprintf(line + n, "%ld,", (long)s->siVADCCurr);                /* 31 */
    n += sprintf(line + n, "%u,",  s->usTemp[0]);                       /* 32 */
    n += sprintf(line + n, "%u,",  s->usTemp[1]);                       /* 33 */
    n += sprintf(line + n, "%u,",  s->usTemp[2]);                       /* 34 */
    n += sprintf(line + n, "%u,",  s->usTemp[3]);                       /* 35 */
    n += sprintf(line + n, "%u,",  s->usTempMin);                       /* 36 */
    n += sprintf(line + n, "%u,",  s->usTempMax);                       /* 37 */
    n += sprintf(line + n, "%u,",  s->usTempI);                         /* 38 */
    n += sprintf(line + n, "%lu,", (unsigned long)s->uiVCHGD);          /* 39 C+电压 */
    n += sprintf(line + n, "0x%08lX,", (unsigned long)s->uiCTOChannel); /* 40 断线位置 */
    n += sprintf(line + n, "0x%08lX\n", (unsigned long)s->uiBALChannel); /* 41 均衡位置 */
    t++;
    return n;
}


static void BMS_save(void *arg)
{
    char path[64];
    FILE *F;
    //如果目录不存在创建目录，权限 
    mkdir(BMS_POINT, 0777);        /* 已存在会返回 -1，不用管 */
    int idx = 1;
    for (idx = 1; idx <= 9999; idx++) {
        snprintf(path, sizeof(path), BMS_POINT "/BMS_%04d.csv", idx);

        F = fopen(path, "rb");    /* 试着只读打开 */
        if (F == NULL) {
            break;                           /* 打不开 = 不存在，就用这个 */
        }
        fclose(F);                       /* 能打开 = 已存在，试下一个 */
        ESP_LOGI(TAG, "%s 已存在，跳过", path);
    }

    F = fopen(path, "wb");
    if (F == NULL) {
        ESP_LOGE(TAG, "创建 %s 失败", path);
        vTaskDelete(NULL);
    }
    /* ---------- 表头开始 ---------- */
    /* UTF-8 BOM：不加这行，Excel 打开中文全是乱码 */
    fputs("\xEF\xBB\xBF", F);

    fprintf(F, "次数,");            /* ts         */
    fprintf(F, "电池包状态,");       /* usPackStatus */
    fprintf(F, "保护状态,");         /* usBatStatus  */
    fprintf(F, "电池包设置,");       /* usPackConfig */
    fprintf(F, "电芯串数,");         /* ucCellNum    */

    for (int i = 1; i <= 20; i++) {
        fprintf(F, "%d芯电压,", i);  /* ssVCell[0..19] */
    }

    fprintf(F, "ssVCellMin,");     /* ssVCellMin   */
    fprintf(F, "ssVCellMax,");     /* ssVCellMax   */
    fprintf(F, "电芯总压,");         /* uiVoltage1   */
    fprintf(F, "BAT+总压,");         /* uiVoltage2   */
    fprintf(F, "siCADCCurr,");     /* siCADCCurr   */
    fprintf(F, "siVADCCurr,");     /* siVADCCurr   */
    fprintf(F, "usTemp1,");   /* usTemp[0]    */
    fprintf(F, "usTemp2,");   /* usTemp[1]    */
    fprintf(F, "usTemp3,");   /* usTemp[2]    */
    fprintf(F, "usTemp4,");   /* usTemp[3]    */
    fprintf(F, "usTempMin,");     /* usTempMin    */
    fprintf(F, "usTempMax,");     /* usTempMax    */
    fprintf(F, "usTempI,");     /* usTempI      */
    fprintf(F, "C+电压,");           /* uiVCHGD      */
    fprintf(F, "断线位置,");         /* uiCTOChannel */
    fprintf(F, "均衡位置\n");         /* uiBALChannel，最后不加逗号 */
/* ---------- 表头结束 ---------- */

    fclose(F);              /* 表头落盘 */
    ESP_LOGI(TAG, "创建 %s", path);

   uint32_t cnt = 0;

    while (1) {
        F = fopen(path, "a");   /* 追加模式 */
    if (F == NULL) {
        ESP_LOGE(TAG, "打开失败");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    for (int i = 0; i < 10; i++) {
        cnt++;
        char line[512];
        int n = build_row(line, &sd_SYSINFOR, cnt);

        fwrite(line, 1, n, F);

        vTaskDelay(pdMS_TO_TICKS(1000));   /* 每秒采一条 */
    }

    fclose(F);              /* ← 这里自动刷盘，不需要 fflush/fsync */
    ESP_LOGI(TAG, "保存一次 %s", path);
    }
}
