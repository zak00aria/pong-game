#ifndef TASKS_H
#define TASKS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ssd1306.h"
#include "../components/inputs.h"
#include "game.h"

void update_display_task(void *pvParameter);
void handle_inputs_task(void *pvParameter);
void play_sounds_task(void *pvParameter);

void game_loop_task(void *pvParameter);

#endif