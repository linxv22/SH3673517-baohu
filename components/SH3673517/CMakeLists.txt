idf_component_register(SRCS "SPIApp.c" "AFE.c" "InitialApp.c"
                    INCLUDE_DIRS "."
                    REQUIRES common 
                    PRIV_REQUIRES esp_driver_spi esp_driver_gpio)
