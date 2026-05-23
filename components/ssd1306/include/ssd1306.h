#ifndef SSD1306_H
#define SSD1306_H

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "driver/i2c.h"
#include "../i2c_utils.h"
#include "font.h"

#define SSD1306_DISPLAY_OFF_CMD 0xAE
#define SSD1306_DISPLAY_ON_CMD  0xAF

enum Text_align {
  LEFT = 1,
  CENTER,
};

struct Screen
{
  const uint8_t width;
  const uint8_t height;
  const uint8_t pages_count;
  uint8_t updated_pages;
  uint8_t updated_columns[8];
  uint8_t *visible_buffer;
  uint8_t *temp_buffer;
  uint8_t *buffer;
};

struct I2C_device
{
  uint8_t address;
  uint8_t i2c_master_num;
};

extern struct Screen screen;
extern struct I2C_device device;

void turn_screen_on(void);
void turn_screen_off(void);

void init_screen(uint8_t address, uint8_t i2c_master_num);

void update_screen(void);

void start_frame(void);

void end_frame(void);

void draw_pixel(uint8_t x, uint8_t y, uint8_t color);
void draw_horizental_line(uint8_t x, uint8_t y, uint8_t w, uint8_t color);
void draw_vertical_line(uint8_t x, uint8_t y, uint8_t w, uint8_t color);
void draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void draw_text(uint8_t x, uint8_t y, char *text, uint8_t color, enum Text_align align, uint8_t scale);

#endif
