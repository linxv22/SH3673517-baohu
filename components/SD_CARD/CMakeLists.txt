idf_component_register(SRCS "sd_init.c"
                    INCLUDE_DIRS "."
                    REQUIRES common fatfs esp_driver_sdmmc)
