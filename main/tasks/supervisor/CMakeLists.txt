idf_component_register(
    SRCS "supervisor.c"
    INCLUDE_DIRS "include"
    REQUIRES limit_switch lcd1602 dc_motor_driver ec11 sg90
)