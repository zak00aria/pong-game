#ifndef INPUTS_H
#define INPUTS_H

#include <stdio.h>
#include "driver/gpio.h"

#define BTN_UP_GPIO 16
#define BTN_RIGHT_GPIO 17
#define BTN_DOWN_GPIO 18
#define BTN_LEFT_GPIO 19
#define BTN_START_GPIO  23

#define INPUT_DEBOUNCER_COUNT_MAX 4

enum Inputs_names {
  btn_start = 0,
  btn_up,
  btn_right,
  btn_down,
  btn_left,
};

struct Inputs {
  uint8_t btn_start;
  uint8_t btn_up;
  uint8_t btn_right;
  uint8_t btn_down;
  uint8_t btn_left;
};

struct Inputs_debouncer {
  uint8_t btn_start;
  uint8_t btn_up;
  uint8_t btn_right;
  uint8_t btn_down;
  uint8_t btn_left;
};

struct Click_event_handlers {
  void (*handlers[5])(void);
  uint8_t inputs_count;
};

extern struct Inputs inputs;
extern struct Inputs_debouncer inputs_debouncer;
extern struct Click_event_handlers click_event_handlers;

void init_inputs(void);
void inputs_handlers(void);

void register_click_event_handler(enum Inputs_names input_name, void (*handler)(void));
void remove_click_event_handler(enum Inputs_names input_name);

#endif