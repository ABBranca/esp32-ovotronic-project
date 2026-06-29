idf_component_register (
    SRCS "ec11.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_pcnt esp_driver_gpio esp_timer
)