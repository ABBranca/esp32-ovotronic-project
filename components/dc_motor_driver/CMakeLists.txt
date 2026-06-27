idf_component_register(
    SRCS "dc_motor_driver.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_ledc
)