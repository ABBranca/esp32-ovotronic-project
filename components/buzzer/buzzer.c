#include "buzzer.h"
#include "driver/ledc.h"

void buzzer_init(const gpio_num_t gpio_num) {
    const ledc_timer_config_t buzzer_ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = 3000, // Frequenza del buzzer
        .duty_resolution = LEDC_TIMER_10_BIT,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&buzzer_ledc_timer);

    const ledc_channel_config_t buzzer_ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_2,
        .timer_sel = LEDC_TIMER_1,
        .gpio_num = gpio_num,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&buzzer_ledc_channel); 

    
    
}