#pragma once

#include <nikola/nikola_math.h>

/// ----------------------------------------------------------------------
/// InputAction
enum InputAction {
  INPUT_ACTION_ACCEPT = 0, 
  INPUT_ACTION_PAUSE, 
  
  INPUT_ACTION_NAVIGATE_UP,
  INPUT_ACTION_NAVIGATE_DOWN,
};
/// InputAction
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Input manager functions

const bool input_manager_action_pressed(const InputAction action);

const nikola::Vec3 input_manager_get_movement_velocity();

/// Input manager functions
/// ----------------------------------------------------------------------
