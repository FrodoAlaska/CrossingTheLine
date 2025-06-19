#pragma once

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// StateType
enum StateType {
  STATE_MENU = 0, 
  STATE_LEVEL, 
  STATE_WON,
  STATE_LOST,

  STATES_MAX = STATE_LOST + 1,
};
/// StateType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// State manager functions

void state_manager_init(nikola::Window* window); 

void state_manager_update(); 

void state_manager_render_hud();

/// State manager functions
/// ----------------------------------------------------------------------
