#include "ssd1306.h"

uint8_t screen_buf[128 * 8] = {0};
uint8_t screen_visible_buf[128 * 8] = {0};
uint8_t screen_temp_buf[128 * 8] = {0};
struct Screen screen = (struct Screen) {
  .width = 128,
  .height = 64,
  .pages_count = 8,
  .updated_pages = 0x00,
  .updated_columns = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  .visible_buffer = screen_visible_buf,
  .temp_buffer = screen_temp_buf,
  .buffer = screen_buf,
};

struct I2C_device device = {};

void init_screen(uint8_t address, uint8_t i2c_master_num)
{
  device.address = address;
  device.i2c_master_num = i2c_master_num;
  turn_screen_on();
}

void update_screen()
{
  for (int16_t i = 0; i < screen.width * screen.pages_count; i++)
  {
    if (screen.temp_buffer[i] == screen.visible_buffer[i]) continue;
    screen.visible_buffer[i] = screen.temp_buffer[i];
    uint8_t column = i / screen.width;
    screen.updated_pages |= 0x01 << column;
    screen.updated_columns[column] |= 0x01 << ((i % screen.width) / 16);
  }

  for (uint8_t y = 0; y < screen.pages_count; y++)
  {
    // if page didn't changed do not send page data to lcd
    if (((screen.updated_pages >> y) & 0x01) == 0) continue;

    for (uint8_t x = 0, address = 0; x < 8; x++, address += 16) {
      // if columns didn't changed do not send data to lcd
      if (((screen.updated_columns[y] >> x) & 0x1) == 0) continue;

      // else send updated page columns data to lcd
      struct I2C_DATA data[3];
      data[0] = (struct I2C_DATA) {
        // set page
        .address = device.address,
        .size = 1,
        .is_cmd = 1,
        .data = (uint8_t[]) {0xB0 | y},
      };
      // set column to 0x20
      data[1] = (struct I2C_DATA) {
        // set column lower nibble
        .address = device.address,
        .size = 1,
        .is_cmd = 1,
        .data = (uint8_t[]) {0x0f & address},
      };
      data[2] = (struct I2C_DATA) {
        // set column upper nibble
        .address = device.address,
        .size = 1,
        .is_cmd = 1,
        .data = (uint8_t[]) {0x10 | (address >> 4)},
      };
      i2c_write_batched_bytes(device.address, device.i2c_master_num, data, 3);
      i2c_write_bytes(device.address, device.i2c_master_num, screen.visible_buffer + y * screen.width + address, 16); // send page pixels data
    }
  }
  screen.updated_pages = 0; // reset updated pages
  memset(screen.updated_columns, 0, 8); // restet updated columns
}

void turn_screen_on(void)
{
  i2c_write_byte(device.address, device.i2c_master_num, SSD1306_DISPLAY_ON_CMD, 1);
}

void start_frame(void)
{
  memset(screen.buffer, 0, screen.width * screen.pages_count);
}

void end_frame(void)
{
  for (int16_t i = 0; i < screen.width * screen.pages_count; i++)
  {
    screen.temp_buffer[i] = screen.buffer[i];
  }
}

void draw_pixel(uint8_t x, uint8_t y, uint8_t color)
{
  if (x >= screen.width || y >= screen.height) return;
  x = screen.width - 1 - x;

  uint8_t column = y / 8;
  uint8_t bit_index = y % 8;
  uint16_t index = column * screen.width + x;

  if (color == 0)
    screen.buffer[index] &= (0x01 << bit_index) ^ 0xff;
  else
    screen.buffer[index] |= 0x01 << bit_index;
}

void draw_horizental_line(uint8_t x, uint8_t y, uint8_t w, uint8_t color)
{
  for (uint8_t i = 0; i <= w; i++)
  {
    draw_pixel(x + i, y, color);
  }
}

void draw_vertical_line(uint8_t x, uint8_t y, uint8_t w, uint8_t color)
{
  for (uint8_t i = 0; i <= w; i++)
  {
    draw_pixel(x, y + i, color);
  }
}

void draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
  for (uint8_t i = 0; i < w; i++)
  {
    draw_pixel(x + i, y, color);
    draw_pixel(x + i, y + h - 1, color);
  }
  for (uint8_t i = 0; i < h; i++)
  {
    draw_pixel(x, y + i, color);
    draw_pixel(x + w - 1, y + i, color);
  }
}

void fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
  for (uint8_t j = 0; j < h; j++)
  {
    for (uint8_t i = 0; i < w; i++)
    {
      draw_pixel(x + i, y + j, color);
    }
  }
}

void draw_text(uint8_t x, uint8_t y, char *text, uint8_t color, enum Text_align align, uint8_t scale)
{
  uint16_t text_length = 0;
  while (text[text_length])
  {
    ++text_length;
  }
  uint8_t x_spacing = scale;
  uint8_t y_spacing = 1;
  if (align == CENTER)
    x -= ((float)text_length / 2) * ((FONT_W * scale) + x_spacing);
  uint8_t font_char_index = 0;
  for (int16_t index = 0; index < text_length; index++)
  {
    font_char_index = (uint8_t)text[index];
    for (uint8_t j = 0; j < FONT_H; j++)
    {
      for (uint8_t i = 0, pixel ; i < FONT_W; i++)
      {
        pixel = (font[font_char_index][j] >> i) & 1;
        if (!pixel) continue;
        fill_rect(x + ((FONT_W - i)*scale), y + (j * scale), scale, scale, pixel ^ !color);
      }
    }
    x += x_spacing + (FONT_W * scale);
  }
}
