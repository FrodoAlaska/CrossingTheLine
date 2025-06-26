#pragma once

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// StateType
enum StateType {
  STATE_MENU = 0, 
  STATE_LEVEL,
  STATE_WON,
  STATE_LOST,
  STATE_CREDITS,

  STATES_MAX = STATE_CREDITS + 1,
};
/// StateType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// State callbacks

using StateInitFunc         = void(*)(nikola::Window* window, const nikola::ResourceID& font_id);
using StateResetFunc        = void(*)();
using StateProcessInputFunc = void(*)();
using StateRenderFunc       = void(*)();

/// State callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// StateDesc
struct StateDesc {
  StateInitFunc init_func          = nullptr;
  StateResetFunc reset_func        = nullptr;
  StateProcessInputFunc input_func = nullptr;
  StateRenderFunc render_func      = nullptr;
};
/// StateDesc
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Menu state functions

void menu_state_init(nikola::Window* window, const nikola::ResourceID& font_id);

void menu_state_reset();

void menu_state_process_input(); 

void menu_state_render();

/// Menu state functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Won state functions

void won_state_init(nikola::Window* window, const nikola::ResourceID& font_id);

void won_state_reset();

void won_state_process_input(); 

void won_state_render();

/// Won state functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Lost state functions

void lost_state_init(nikola::Window* window, const nikola::ResourceID& font_id);

void lost_state_reset();

void lost_state_process_input(); 

void lost_state_render();

/// Lost state functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Credits state functions

void credits_state_init(nikola::Window* window, const nikola::ResourceID& font_id);

void credits_state_reset();

void credits_state_process_input(); 

void credits_state_render();

/// Credits state functions
/// ----------------------------------------------------------------------
