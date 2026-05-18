#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "ssd1306.h"
#include "../components/inputs.h"
#include "../components/buzzer.h"

enum Game_state {
  MENU = 0,
  PAUSED,
  PLAYING,
  COMPLETED,
};

enum Game_mode {
  EASY = 0,
  MEDIUM,
  HARD,
};

struct Game_mode_constraints {
  uint8_t ball_speed_x;
  void (*move_computer_pad)(void);
};

struct Game {
  enum Game_state state;
  enum Game_mode mode;
  uint16_t score[2];
  uint8_t is_state_initialized;
};

struct Menu_action {
  char *title;
  void (*action)(void);
};

struct Menu {
  char *title;
  struct Menu_action *actions;
  uint8_t selected_action_index;
  uint8_t actions_count;
};

struct Tone {
  uint32_t freq;
  uint32_t dur;
};

struct Game_tones {
  void (*pad_hit)(void);
  void (*border_hit)(void);
  void (*computer_score)(void);
  void (*player_score)(void);
  void (*win)(void);
  void (*lost)(void);
  void (*key_press)(void);
  void (*select_key_press)(void);
};

struct Ball {
  uint8_t size;
  uint8_t x;
  uint8_t y;
  int8_t speed_x;
  int8_t speed_y;
};

struct Pad {
  uint8_t width;
  uint8_t height;
  uint8_t x;
  uint8_t y;
  int8_t speed_x;
  int8_t speed_y;
};

extern struct Game game;
extern struct Game_tones game_tones;
extern struct Tone tones_queue[10];
extern QueueHandle_t tones_queue_handle;

void game_loop(void);

void pause_game(void);
void continue_game(void);
void start_game(enum Game_mode game_mode);
void restart_game(void);
void exit_game(void);

void draw_main_menu(void);
void update_main_menu(void);

void draw_game(void);
void update_game(void);

void draw_pause_menu(void);
void update_pause_menu(void);

void draw_end_game_menu(void);
void update_end_game_menu(void);

void menu_go_up(struct Menu *menu);
void menu_go_down(struct Menu *menu);
void menu_select_action(struct Menu *menu);

void pause_game(void);

void start_game_easy_mode(void);
void start_game_medium_mode(void);
void start_game_hard_mode(void);

void move_ball(void);
void move_computer_pad_easy(void);
void move_computer_pad_medium(void);
void move_computer_pad_hard(void);
int8_t get_ball_reflect_angle(struct Pad *pad);

void play_tone(uint32_t freq, uint32_t dur);

void play_pad_hit_sound(void);
void play_border_hit_sound(void);
void play_computer_score_sound(void);
void play_player_score_sound(void);
void play_win_sound(void);
void play_lost_sound(void);
void play_key_press_sound(void);
void play_select_key_press_sound(void);

#endif
