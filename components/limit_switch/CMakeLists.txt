idf_component_register(
    SRCS "limit_switch.c"
    INCLUDE_DIRS "include" "../../main/tasks/supervisor/include"
    REQUIRES esp_driver_gpio
)
