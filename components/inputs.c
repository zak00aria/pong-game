#include "inputs.h"

uint8_t clicks_register = 0;

struct Inputs inputs = {
  .btn_start = 0,
  .btn_up = 0,
  .btn_right = 0,
  .btn_down = 0,
  .btn_left = 0,
};

struct Inputs_debouncer inputs_debouncer = {0};

struct Click_event_handlers click_event_handlers = {
  .handlers = {NULL, NULL, NULL, NULL, NULL},
  .inputs_count = 5,
};

void init_inputs(void)
{
  gpio_reset_pin(BTN_UP_GPIO);
  gpio_set_direction(BTN_UP_GPIO, GPIO_MODE_INPUT);
  gpio_set_pull_mode(BTN_UP_GPIO, GPIO_PULLDOWN_ENABLE);

  gpio_reset_pin(BTN_RIGHT_GPIO);
  gpio_set_direction(BTN_RIGHT_GPIO, GPIO_MODE_INPUT);
  gpio_set_pull_mode(BTN_RIGHT_GPIO, GPIO_PULLDOWN_ENABLE);

  gpio_reset_pin(BTN_DOWN_GPIO);
  gpio_set_direction(BTN_DOWN_GPIO, GPIO_MODE_INPUT);
  gpio_set_pull_mode(BTN_DOWN_GPIO, GPIO_PULLDOWN_ENABLE);

  gpio_reset_pin(BTN_LEFT_GPIO);
  gpio_set_direction(BTN_LEFT_GPIO, GPIO_MODE_INPUT);
  gpio_set_pull_mode(BTN_LEFT_GPIO, GPIO_PULLDOWN_ENABLE);

  gpio_reset_pin(BTN_START_GPIO);
  gpio_set_direction(BTN_START_GPIO, GPIO_MODE_INPUT);
  gpio_set_pull_mode(BTN_START_GPIO, GPIO_PULLDOWN_ENABLE);
}

void debounce_btn(enum Inputs_names input_name, int btn_gpio_pin, uint8_t *btn_level, uint8_t *btn_debounce_counter)
{
  if (*btn_debounce_counter > 0 && *btn_debounce_counter < INPUT_DEBOUNCER_COUNT_MAX)
  {
    ++(*btn_debounce_counter);
    return;
  }

  uint8_t new_btn_level = gpio_get_level(btn_gpio_pin);

  if (new_btn_level == *btn_level)
    return;

  if (!new_btn_level)
    clicks_register |= 1 << input_name;

  *btn_debounce_counter = 1;
  *btn_level = new_btn_level;
}

void inputs_handlers(void)
{
  debounce_btn(btn_start, BTN_START_GPIO, &(inputs.btn_start), &(inputs_debouncer.btn_start));
  debounce_btn(btn_up, BTN_UP_GPIO, &(inputs.btn_up), &(inputs_debouncer.btn_up));
  debounce_btn(btn_right, BTN_RIGHT_GPIO, &(inputs.btn_right), &(inputs_debouncer.btn_right));
  debounce_btn(btn_down, BTN_DOWN_GPIO, &(inputs.btn_down), &(inputs_debouncer.btn_down));
  debounce_btn(btn_left, BTN_LEFT_GPIO, &(inputs.btn_left), &(inputs_debouncer.btn_left));
  for(uint8_t i=0;i<5;i++)
  {
    if(click_event_handlers.handlers[i]!=NULL && (clicks_register>>i) & 1)
      click_event_handlers.handlers[i]();
  }
  clicks_register = 0;
}

void register_click_event_handler(enum Inputs_names input_name, void (*handler)(void))
{
  click_event_handlers.handlers[(size_t)input_name] = handler;
}

void remove_click_event_handler(enum Inputs_names input_name)
{
  click_event_handlers.handlers[(size_t)input_name] = 0;
}
