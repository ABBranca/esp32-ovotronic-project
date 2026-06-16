#include "tmp102q1.h"
#include "driver/i2c_master.h"

i2c_device_config_t tmp102_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = TMP102_ADDRESS,
    .scl_speed_hz = 100000,
};
