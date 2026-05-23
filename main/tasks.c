#include "tasks.h"

void update_display_task(void *pvParameter)
{
  while (1)
  {
    update_screen();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void handle_inputs_task(void *pvParameter)
{
  while (1)
  {
    inputs_handlers();
    vTaskDelay(30 / portTICK_PERIOD_MS);
  }
}

void game_loop_task(void *pvParameter)
{
  game_loop();
}

void play_sounds_task(void *pvParameter)
{
  struct Tone tone;
  while (1)
  {
    if (xQueueReceive(tones_queue_handle, &tone, portMAX_DELAY) == pdTRUE) {
      make_tone(tone.freq, tone.dur);
    }
  }
}
