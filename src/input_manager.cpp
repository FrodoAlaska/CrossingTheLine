#include "input_manager.h"

#include <nikola/nikola_math.h>
#include <nikola/nikola_input.h>

/// ----------------------------------------------------------------------
/// Private functions

static const bool get_key_action_pressed(const InputAction action) {
  switch(action) {
    case INPUT_ACTION_ACCEPT:
      return nikola::input_key_pressed(nikola::KEY_ENTER);
    case INPUT_ACTION_PAUSE:
      return nikola::input_key_pressed(nikola::KEY_P);
    case INPUT_ACTION_NAVIGATE_UP:
      return nikola::input_key_pressed(nikola::KEY_UP) || nikola::input_key_pressed(nikola::KEY_W);
    case INPUT_ACTION_NAVIGATE_DOWN:
      return nikola::input_key_pressed(nikola::KEY_DOWN) || nikola::input_key_pressed(nikola::KEY_S);
    default:
      return false;
  } 

  return false;
}

static const bool get_gamepad_action_pressed(const InputAction action) {
  switch(action) {
    case INPUT_ACTION_ACCEPT:
      return nikola::input_gamepad_button_pressed(nikola::JOYSTICK_ID_0, nikola::GAMEPAD_BUTTON_CROSS);
    case INPUT_ACTION_PAUSE:
      return nikola::input_gamepad_button_pressed(nikola::JOYSTICK_ID_0, nikola::GAMEPAD_BUTTON_START);
    case INPUT_ACTION_NAVIGATE_UP:
      return nikola::input_gamepad_button_pressed(nikola::JOYSTICK_ID_0, nikola::GAMEPAD_BUTTON_DPAD_UP);
    case INPUT_ACTION_NAVIGATE_DOWN:
      return nikola::input_gamepad_button_pressed(nikola::JOYSTICK_ID_0, nikola::GAMEPAD_BUTTON_DPAD_DOWN);
    default:
      return false;
  } 

  return false;
}

static const nikola::Vec3 get_key_movement_velocity() {
  nikola::Vec3 velocity(0.0f); 

  if(nikola::input_key_down(nikola::KEY_W)) {
    velocity.x = 1.0f;
  }
  // Move backwards
  else if(nikola::input_key_down(nikola::KEY_S)) {
    velocity.x = -1.0f;
  }
 
  // Move right
  if(nikola::input_key_down(nikola::KEY_A)) {
    velocity.z = -1.0f;
  }
  // Move left
  else if(nikola::input_key_down(nikola::KEY_D)) {
    velocity.z = 1.0f;
  }

  return velocity;
}

static const nikola::Vec3 get_gamepad_movement_velocity() {
  float x, z;
  nikola::input_gamepad_axis_value(nikola::JOYSTICK_ID_0, nikola::GAMEPAD_AXIS_LEFT, &z, &x);

  return nikola::Vec3(-x, 0.0f, z);
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Input manager functions

const bool input_manager_action_pressed(const InputAction action) {
  if(nikola::input_gamepad_connected(nikola::JOYSTICK_ID_0)) {
    return get_gamepad_action_pressed(action);
  }

  return get_key_action_pressed(action);
}

const nikola::Vec3 input_manager_get_movement_velocity() {
  if(nikola::input_gamepad_connected(nikola::JOYSTICK_ID_0)) {
    return get_gamepad_movement_velocity();
  }

  return get_key_movement_velocity();
}

/// Input manager functions
/// -----------------------------------------------------------------------
