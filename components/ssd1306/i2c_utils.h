#ifndef I2C_UTILS_H
#define I2C_UTILS_H

#include "freertos/FreeRTOS.h"
#include "driver/i2c.h"

#define I2C_TIMEOUT_MS 20

struct I2C_DATA {
  uint16_t size; // in bytes
  uint8_t address; // device address
  uint8_t is_cmd; // 1 if it's a command, 0 if it's data
  const uint8_t *data; // pointer to data bytes array
};

esp_err_t i2c_write_byte(
  uint8_t device_addr, // device address
  uint8_t i2c_master_num,
  uint8_t data, // data or command to send
  uint8_t is_cmd_mode // 1 if it's a command, 0 if it's data
);

esp_err_t i2c_write_batched_bytes(
  uint8_t device_addr,
  uint8_t i2c_master_num,
  struct I2C_DATA *i2c_data,
  uint16_t len
);

esp_err_t i2c_write_bytes(
  uint8_t device_addr,
  uint8_t i2c_master_num,
  uint8_t *data,
  uint16_t size
);

#endif
