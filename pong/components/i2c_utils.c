#include "i2c_utils.h"

esp_err_t i2c_write_byte(
  uint8_t device_addr,
  uint8_t i2c_master_num,
  uint8_t data,
  uint8_t is_cmd_mode
)
{
  i2c_cmd_handle_t i2c_handle = i2c_cmd_link_create();

  i2c_master_start(i2c_handle);
  i2c_master_write_byte(i2c_handle, (device_addr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(i2c_handle, is_cmd_mode ? 0x00 : 0x40, true);
  i2c_master_write_byte(i2c_handle, data, true);
  i2c_master_stop(i2c_handle);

  esp_err_t ret = i2c_master_cmd_begin(i2c_master_num, i2c_handle, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(i2c_handle);

  if (ret != ESP_OK)
  {
    printf("I2C write failed: %s", esp_err_to_name(ret));
  }

  return ret;
}

esp_err_t i2c_write_bytes(
  uint8_t device_addr,
  uint8_t i2c_master_num,
  uint8_t *data,
  uint16_t size
)
{
  i2c_cmd_handle_t i2c_handle = i2c_cmd_link_create();

  i2c_master_start(i2c_handle);

  i2c_master_write_byte(i2c_handle, (device_addr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(i2c_handle, 0x40, true);
  for (uint16_t i = 0; i < size; i++)
    i2c_master_write_byte(i2c_handle, data[i], true);

  i2c_master_stop(i2c_handle);

  esp_err_t ret = i2c_master_cmd_begin(i2c_master_num, i2c_handle, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(i2c_handle);

  return ret;
}

esp_err_t i2c_write_batched_bytes(
  uint8_t device_addr,
  uint8_t i2c_master_num,
  struct I2C_DATA *i2c_data,
  uint16_t len
)
{
  i2c_cmd_handle_t i2c_handle = i2c_cmd_link_create();

  i2c_master_start(i2c_handle);

  for (uint16_t i = 0; i < len; i++)
  {
    i2c_master_write_byte(i2c_handle, (i2c_data[i].address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(i2c_handle, i2c_data[i].is_cmd ? 0x00 : 0x40, true);
    for (uint16_t j = 0; j < i2c_data[i].size; j++)
      i2c_master_write_byte(i2c_handle, i2c_data[i].data[j], true);
  }

  i2c_master_stop(i2c_handle);

  esp_err_t ret = i2c_master_cmd_begin(i2c_master_num, i2c_handle, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(i2c_handle);

  return ret;
}
