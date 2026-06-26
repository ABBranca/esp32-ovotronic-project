idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES i2c_bus tmp102q1 lcd1602 sg90 buzzer limit_switch)