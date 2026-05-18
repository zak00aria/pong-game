#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#include "ssd1306.h"
#include "../components/inputs.h"
#include "../components/buzzer.h"
#include "tasks.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 800000

#define SSD1306_ADDR (uint8_t)0x3C

esp_err_t i2c_master_init(void)
{
  i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  i2c_param_config(I2C_MASTER_NUM, &conf);
  return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void app_main(void)
{
  i2c_master_init();

  init_screen(SSD1306_ADDR, I2C_MASTER_NUM);
  init_inputs();
  init_buzzer(4);

  xTaskCreate(update_display_task, "update_display", 2048, NULL, 2, NULL);
  xTaskCreate(handle_inputs_task, "handle_inputs", 2048, NULL, 3, NULL);
  xTaskCreate(play_sounds_task, "play_sounds", 2048, NULL, 1, NULL);
  xTaskCreate(game_loop_task, "game_loop", 4096, NULL, 3, NULL);
}
