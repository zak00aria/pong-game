#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "driver/ledc.h"

void init_buzzer(size_t gpio_pin);
void make_tone(uint32_t freq, uint32_t dur);

#endif