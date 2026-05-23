#include "game.h"

#define BALL_REFLECT_ANGLES_LENGTH 7
int8_t ball_reflect_angles[BALL_REFLECT_ANGLES_LENGTH] = {-5, -2, -1, 0, 1, 2, 5};

int16_t dist_y = 0;

QueueHandle_t tones_queue_handle = NULL;

struct Game game = {
  .state = MENU,
  .mode = EASY,
  .is_state_initialized = 0,
  .score = {0, 0},
};

struct Game_tones game_tones = {
  .pad_hit = play_pad_hit_sound,
  .border_hit = play_border_hit_sound,
  .computer_score = play_computer_score_sound,
  .player_score = play_player_score_sound,
  .win = play_win_sound,
  .lost = play_lost_sound,
  .key_press = play_key_press_sound,
  .select_key_press = play_select_key_press_sound,
};

struct Game_mode_constraints game_modes_constraints[3] = {
  {
    .ball_speed_x = 3,
    .move_computer_pad = move_computer_pad_easy,
  },
  {
    .ball_speed_x = 4,
    .move_computer_pad = move_computer_pad_medium,
  },
  {
    .ball_speed_x = 5,
    .move_computer_pad = move_computer_pad_hard,
  }
};

struct Menu_action main_menu_actions[3] = {
  {
    .title = "EASY",
    .action = start_game_easy_mode,
  },
  {
    .title = "MEDIUM",
    .action = start_game_medium_mode,
  },
  {
    .title = "HARD",
    .action = start_game_hard_mode,
  },
};

struct Menu main_menu = {
  .title = "PONG GAME",
  .actions_count = 3,
  .actions = main_menu_actions,
  .selected_action_index = 0,
};

struct Menu_action pause_menu_actions[3] = {
  {
    .title = "CONTINUE",
    .action = continue_game,
  },
  {
    .title = "RESTART",
    .action = restart_game,
  },
  {
    .title = "EXIT",
    .action = exit_game,
  },
};

struct Menu pause_menu = {
  .title = "PAUSED",
  .actions_count = 3,
  .actions = pause_menu_actions,
  .selected_action_index = 0,
};

struct Menu_action end_game_menu_actions[3] = {
  {
    .title = "PLAY AGAIN",
    .action = restart_game,
  },
  {
    .title = "EXIT",
    .action = exit_game,
  },
};

struct Menu end_game_menu = {
  .title = "",
  .actions_count = 2,
  .actions = end_game_menu_actions,
  .selected_action_index = 0,
};

char computer_score[6] = "0"; // current computer score will be converted to string and stored here.
char player_score[6] = "0"; // current player score will be converted to string and stored here.

struct Ball ball = {0};
struct Pad computer_pad = {0};
struct Pad player_pad = {0};

void game_loop(void)
{
  tones_queue_handle = xQueueCreate(10, sizeof(struct Tone));
  while (1)
  {
    switch (game.state)
    {
      case MENU:
        start_frame();

        update_main_menu();
        draw_main_menu();

        end_frame();
        break;
      case PLAYING:
        start_frame();

        update_game();
        draw_game();

        end_frame();

        if (game.score[0] >= 10 || game.score[1] >= 10) {
          game.state = COMPLETED;
          game.is_state_initialized = 0;
        };
        break;
      case PAUSED:
        start_frame();

        draw_pause_menu();
        update_pause_menu();

        end_frame();
        break;
      case COMPLETED:
        start_frame();

        draw_end_game_menu();
        update_end_game_menu();

        end_frame();
        break;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }

}

void draw_game(void)
{
  // draw game board
  draw_horizental_line(0, 0, screen.width, 1);
  draw_horizental_line(0, screen.height - 1, screen.width, 1);
  draw_vertical_line(screen.width >> 1, 0, screen.height, 1);

  // draw score
  draw_text((screen.width >> 1) - 20, 6, computer_score, 1, CENTER, 1);
  draw_text((screen.width >> 1) + 10, 6, player_score, 1, CENTER, 1);

  // draw ball and player/computer pads
  fill_rect(ball.x, ball.y, ball.size, ball.size, 1);

  // draw player/computer pads
  fill_rect(computer_pad.x, computer_pad.y, computer_pad.width, computer_pad.height, 1);
  fill_rect(player_pad.x, player_pad.y, player_pad.width, player_pad.height, 1);
}

void update_game(void)
{
  if (!game.is_state_initialized)
  {
    game.score[0] = 0;
    game.score[1] = 0;
    computer_score[0] = '0';
    computer_score[1] = '\0';
    player_score[0] = '0';
    player_score[1] = '\0';
    ball.size = 2;
    ball.x = screen.width >> 1;
    ball.y = screen.height >> 1;
    ball.speed_x = game_modes_constraints[game.mode].ball_speed_x;
    ball.speed_y = 0;

    computer_pad.width = 2;
    computer_pad.height = 20;
    computer_pad.x = 4;
    computer_pad.y = (screen.height >> 1) - 10;
    computer_pad.speed_x = 0;
    computer_pad.speed_y = 2;

    player_pad.width = 2;
    player_pad.height = 20;
    player_pad.x = screen.width - 4 - 2;
    player_pad.y = (screen.height >> 1) - 10;
    player_pad.speed_x = 0;
    player_pad.speed_y = 2;

    register_click_event_handler(btn_start, pause_game);
    remove_click_event_handler(btn_up);
    remove_click_event_handler(btn_right);
    remove_click_event_handler(btn_down);
    remove_click_event_handler(btn_left);
    game.is_state_initialized = 1;
  }

  // update ball position
  move_ball();

  // update computer pad position
  game_modes_constraints[game.mode].move_computer_pad();

  // update player pad position
  if (inputs.btn_up && player_pad.y > player_pad.speed_y)
    player_pad.y -= player_pad.speed_y;
  else if (inputs.btn_down && player_pad.y + player_pad.height < screen.height - player_pad.speed_y)
    player_pad.y += player_pad.speed_y;

  // check if computer or player scored
  if (ball.x + ball.size >= screen.width || ball.x <= 1)
  {
    if (ball.x <= 1)
    {
      game.score[1] += 1;
      sprintf(player_score, "%d", game.score[1]);
      game_tones.player_score();
    }
    else
    {
      game.score[0] += 1;
      sprintf(computer_score, "%d", game.score[0]);
      game_tones.computer_score();
    }
    ball.x = screen.width >> 1;
    ball.y = screen.height >> 1;
    ball.speed_y = 0;
  }

  // check ball collision with player pad
  if ( ball.y + ball.size >= player_pad.y &&
       ball.y <= player_pad.y + player_pad.height )
  {
    if ( ball.speed_x > 0 &&
         ball.x + ball.size - player_pad.x <= player_pad.width &&
         ball.x + ball.size - player_pad.x >= 0 )
    {
      ball.speed_x = -ball.speed_x;
      // decide ball direction on y axis
      ball.speed_y = get_ball_reflect_angle(&player_pad);
      game_tones.pad_hit();
    }
  }

  // check ball collision with computer pad
  if ( ball.y + ball.size >= computer_pad.y &&
       ball.y <= computer_pad.y + computer_pad.height )
  {
    if ( ball.speed_x < 0 &&
         ball.x - computer_pad.x <= computer_pad.width &&
         ball.x - computer_pad.x >= 0 )
    {
      ball.speed_x = -ball.speed_x;
      // decide ball direction on y axis
      ball.speed_y = get_ball_reflect_angle(&computer_pad);
      game_tones.pad_hit();
    }
  }

  // check ball collision with top and bottom borders of the board
  if (ball.y + ball.size >= screen.height - 1 || ball.y < 1)
  {
    ball.speed_y = -ball.speed_y;
    game_tones.border_hit();
  }
}


void menu_go_up(struct Menu *menu)
{
  if (menu->selected_action_index >= 1)
  {
    --menu->selected_action_index;
    game_tones.key_press();
  }
}

void menu_go_down(struct Menu *menu)
{
  if (menu->selected_action_index < menu->actions_count - 1)
  {
    ++menu->selected_action_index;
    game_tones.key_press();
  }
}

void menu_select_action(struct Menu *menu)
{
  menu->actions[menu->selected_action_index].action();
  game_tones.select_key_press();
}

void main_menu_go_up(void)
{
  menu_go_up(&main_menu);
}
void main_menu_go_down(void)
{
  menu_go_down(&main_menu);
}
void main_menu_select_action(void)
{
  menu_select_action(&main_menu);
}

void pause_menu_go_up(void)
{
  menu_go_up(&pause_menu);
}
void pause_menu_go_down(void)
{
  menu_go_down(&pause_menu);
}
void pause_menu_select_action(void)
{
  menu_select_action(&pause_menu);
}

void end_game_menu_go_up(void)
{
  menu_go_up(&end_game_menu);
}
void end_game_menu_go_down(void)
{
  menu_go_down(&end_game_menu);
}
void end_game_menu_select_action(void)
{
  menu_select_action(&end_game_menu);
}

void draw_main_menu(void)
{
  draw_rect(0, 0, screen.width - 1, screen.height - 1, 1);
  draw_text(screen.width >> 1, 4, main_menu.title, 1, CENTER, 1);
  for (uint8_t i = 0; i < main_menu.actions_count; i++)
  {
    uint16_t menu_action_title_length = 0;
    while (main_menu.actions[i].title[menu_action_title_length])
    {
      ++menu_action_title_length;
    }
    if (main_menu.selected_action_index == i)
      fill_rect((screen.width >> 1) - (((menu_action_title_length) * (FONT_W + 1) + 4) >> 1), 16 + (i * 11), (menu_action_title_length) * (FONT_W + 1) + 4, (FONT_H + 4), 1);
    draw_text(screen.width >> 1, 16 + (i * 11) + 2, main_menu.actions[i].title, main_menu.selected_action_index != i, CENTER, 1);
  }
}

void update_main_menu(void)
{
  if (!game.is_state_initialized)
  {
    register_click_event_handler(btn_start, main_menu_select_action);
    register_click_event_handler(btn_up, main_menu_go_up);
    register_click_event_handler(btn_down, main_menu_go_down);
    remove_click_event_handler(btn_right);
    remove_click_event_handler(btn_left);
    game.is_state_initialized = 1;
  }
}

void draw_pause_menu(void)
{
  draw_rect(0, 0, screen.width - 1, screen.height - 1, 1);
  draw_text(screen.width >> 1, 4, pause_menu.title, 1, CENTER, 1);
  for (uint8_t i = 0; i < pause_menu.actions_count; i++)
  {
    uint16_t menu_action_title_length = 0;
    while (pause_menu.actions[i].title[menu_action_title_length])
    {
      ++menu_action_title_length;
    }
    if (pause_menu.selected_action_index == i)
      fill_rect((screen.width >> 1) - (((menu_action_title_length) * (FONT_W + 1) + 4) >> 1), 16 + (i * 11), (menu_action_title_length) * (FONT_W + 1) + 4, (FONT_H + 4), 1);
    draw_text(screen.width >> 1, 16 + (i * 11) + 2, pause_menu.actions[i].title, pause_menu.selected_action_index != i, CENTER, 1);
  }
}

void update_pause_menu(void)
{
  if (!game.is_state_initialized)
  {
    register_click_event_handler(btn_start, pause_menu_select_action);
    register_click_event_handler(btn_up, pause_menu_go_up);
    register_click_event_handler(btn_down, pause_menu_go_down);
    remove_click_event_handler(btn_right);
    remove_click_event_handler(btn_left);
    game.is_state_initialized = 1;
  }
}

void draw_end_game_menu(void)
{
  draw_rect(0, 0, screen.width - 1, screen.height - 1, 1);
  // draw result
  draw_text((screen.width >> 1) - 5, 8, game.score[0] > game.score[1] ? " YOU LOST" : "YOU WIN", 1, CENTER, 1);
  draw_text((screen.width >> 1) - 10, 18, computer_score, 1, CENTER, 1);
  draw_text((screen.width >> 1) + 10, 18, player_score, 1, CENTER, 1);

  for (uint8_t i = 0; i < end_game_menu.actions_count; i++)
  {
    uint16_t menu_action_title_length = 0;
    while (end_game_menu.actions[i].title[menu_action_title_length])
    {
      ++menu_action_title_length;
    }
    if (end_game_menu.selected_action_index == i)
      fill_rect((screen.width >> 1) - (((menu_action_title_length) * (FONT_W + 1) + 4) >> 1), 30 + (i * 11), (menu_action_title_length) * (FONT_W + 1) + 4, (FONT_H + 4), 1);
    draw_text(screen.width >> 1, 30 + (i * 11) + 2, end_game_menu.actions[i].title, end_game_menu.selected_action_index != i, CENTER, 1);
  }
}

void update_end_game_menu(void)
{
  if (!game.is_state_initialized)
  {
    register_click_event_handler(btn_start, end_game_menu_select_action);
    register_click_event_handler(btn_up, end_game_menu_go_up);
    register_click_event_handler(btn_down, end_game_menu_go_down);
    remove_click_event_handler(btn_right);
    remove_click_event_handler(btn_left);
    game.is_state_initialized = 1;
  }
}

void pause_game(void) {
  game.is_state_initialized = 0;
  game.state = PAUSED;
  game_tones.select_key_press();
}

void continue_game(void) {
  register_click_event_handler(btn_start, pause_game);
  remove_click_event_handler(btn_up);
  remove_click_event_handler(btn_right);
  remove_click_event_handler(btn_down);
  remove_click_event_handler(btn_left);
  game.state = PLAYING;
}

void start_game(enum Game_mode game_mode)
{
  game.is_state_initialized = 0;
  game.mode = game_mode;
  game.state = PLAYING;
}

void restart_game(void) {
  start_game(game.mode);
}

void exit_game(void) {
  game.is_state_initialized = 0;
  game.mode = EASY;
  game.state = MENU;
}

void start_game_easy_mode (void)
{
  start_game(EASY);
}

void start_game_medium_mode (void)
{
  start_game(MEDIUM);
}

void start_game_hard_mode (void)
{
  start_game(HARD);
}

void move_pad_up(struct Pad *pad)
{
  if (pad->y >= pad->speed_y)
    pad->y -= pad->speed_y;
  else
    computer_pad.y = 0;
}

void move_pad_down(struct Pad *pad)
{
  if (screen.height - pad->y - pad->height - 1 >= pad->speed_y)
    pad->y += pad->speed_y;
  else
    pad->y = screen.height - pad->height - 1;
}

float get_line_point_y(uint8_t x, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
  float a = (float)(y2 - y1) / (x2 - x1);
  float b = y1 - a * x1;
  return (float)(a * x + b);
}

void move_computer_pad_easy (void)
{
  if (ball.speed_x < 0 && ball.x <= screen.width >> 1)
  {
    dist_y = get_line_point_y(computer_pad.x + computer_pad.width, ball.x - ball.speed_x, ball.y - ball.speed_y, ball.x, ball.y);
    dist_y += - (computer_pad.height >> 1) + ((player_pad.y + (player_pad.height >> 1) > (screen.height >> 1)) ? ((computer_pad.height / BALL_REFLECT_ANGLES_LENGTH) << 1) : -((computer_pad.height / BALL_REFLECT_ANGLES_LENGTH) << 1));
    if (dist_y < 0 || dist_y > screen.height + 1)
      dist_y = ball.y;
    if (dist_y > computer_pad.y && computer_pad.y + computer_pad.height  < screen.height - 1)
      move_pad_down(&computer_pad);
    else if (dist_y < computer_pad.y && computer_pad.y > 0)
      move_pad_up(&computer_pad);
  }
}

void move_computer_pad_medium (void)
{
  if (ball.speed_x < 0)
  {
    dist_y = get_line_point_y(computer_pad.x + computer_pad.width, ball.x - ball.speed_x, ball.y - ball.speed_y, ball.x, ball.y);
    dist_y += - (computer_pad.height >> 1) + ((player_pad.y + (player_pad.height >> 1) > (screen.height >> 1)) ? ((computer_pad.height / BALL_REFLECT_ANGLES_LENGTH) << 1) : -((computer_pad.height / BALL_REFLECT_ANGLES_LENGTH) << 1));
    if (dist_y < 0 || dist_y > screen.height + 1)
      dist_y = ball.y;
    if (dist_y > computer_pad.y && computer_pad.y + computer_pad.height  < screen.height - 1)
      move_pad_down(&computer_pad);
    else if (dist_y < computer_pad.y && computer_pad.y > 0)
      move_pad_up(&computer_pad);
  }
}

void move_computer_pad_hard(void)
{
  if (ball.speed_x < 0)
  {
    dist_y = get_line_point_y(computer_pad.x + computer_pad.width, ball.x - ball.speed_x, ball.y - ball.speed_y, ball.x, ball.y);
    dist_y += - (computer_pad.height >> 1) + ((player_pad.y + (player_pad.height >> 1) > (screen.height >> 1)) ? ((computer_pad.height / BALL_REFLECT_ANGLES_LENGTH) << 1) : -((computer_pad.height / BALL_REFLECT_ANGLES_LENGTH) << 1));
    if (dist_y < 0 || dist_y > screen.height + 1)
      dist_y = ball.y;
    if (dist_y > computer_pad.y && computer_pad.y + computer_pad.height  < screen.height - 1)
      move_pad_down(&computer_pad);
    else if (dist_y < computer_pad.y && computer_pad.y > 0)
      move_pad_up(&computer_pad);
    return;
  }
  if (computer_pad.y + (computer_pad.height >> 1) > screen.height >> 1)
    move_pad_up(&computer_pad);
  else if (computer_pad.y + (computer_pad.height >> 1) < screen.height >> 1)
    move_pad_down(&computer_pad);
}

int8_t get_ball_reflect_angle(struct Pad *pad)
{
  float step = (float)pad->height / (BALL_REFLECT_ANGLES_LENGTH - 1);
  if (ball.y < pad->y)
    return ball_reflect_angles[(size_t)((ball.y + ball.size - pad->y) / step)];
  return ball_reflect_angles[(size_t)((ball.y - pad->y) / step)];
}

void move_ball(void)
{
  uint8_t new_ball_x = ball.x + ball.speed_x;
  uint8_t new_ball_y = ball.y + ball.speed_y;
  if (ball.speed_x < 0 && ball.x < -ball.speed_x)
    new_ball_x = 0;
  else if (ball.speed_x > 0 && ball.x + ball.size > screen.width - ball.speed_x)
    new_ball_x = screen.width - ball.size;
  float a = (float)(new_ball_y - ball.y) / (new_ball_x - ball.x);
  float b = ball.y - a * ball.x;
  float y;
  if (ball.speed_x > 0 && ball.x <= player_pad.x && new_ball_x + ball.size > player_pad.x + player_pad.width)
  {
    y = a * player_pad.x + b;
    if (y > player_pad.y - ball.size && y < player_pad.y + player_pad.height)
    {
      ball.x = player_pad.x - ball.size;
      ball.y = (uint8_t)y;
      return;
    }
  }
  if (ball.speed_x < 0 && ball.x >= computer_pad.x && new_ball_x < computer_pad.x + computer_pad.width)
  {
    y = a * (computer_pad.x + computer_pad.width) + b;
    if (y > computer_pad.y - ball.size && y < computer_pad.y + computer_pad.height)
    {
      ball.x = computer_pad.x + computer_pad.width;
      ball.y = (uint8_t)y;
      return;
    }
  }
  ball.x = new_ball_x;
  ball.y = new_ball_y;
}

void play_tone(uint32_t freq, uint32_t dur)
{
  struct Tone tone = {
    .freq = freq,
    .dur = dur,
  };
  xQueueSend(tones_queue_handle, &tone, 0);
}

void play_pad_hit_sound()
{
  play_tone(400, 20);
}
void play_border_hit_sound()
{
  play_tone(300, 20);
}
void play_computer_score_sound()
{
  play_tone(400, 40);
  play_tone(300, 40);
  play_tone(200, 40);
}
void play_player_score_sound()
{
  play_tone(1200, 40);
  play_tone(1500, 40);
  play_tone(1800, 40);
}
void play_win_sound()
{
  play_tone(1800, 40);
}
void play_lost_sound()
{
  play_tone(200, 40);
}
void play_key_press_sound()
{
  play_tone(1000, 10);
}
void play_select_key_press_sound()
{
  play_tone(700, 10);
  play_tone(1200, 10);
  play_tone(700, 10);
}
