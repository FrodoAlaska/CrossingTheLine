#include "entity.h"
#include "levels/level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Consts

const float PLAYER_SPEED = 15.0f;

/// Consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Globals

static bool s_player_can_move = true;

/// Globals
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Player functions

void player_create(Entity* player, Level* lvl, const nikola::Vec3& start_pos) {
  // Entity init
  entity_create(player, 
                lvl, 
                start_pos, 
                nikola::Vec3(1.2f, 3.0f, 1.2f), 
                ENTITY_PLAYER, 
                nikola::PHYSICS_BODY_KINEMATIC);

  s_player_can_move = true;
}

void player_update(Entity& player) {
  if(!player.is_active) {
    return;
  }

  // Movement

  nikola::Vec3 velocity = nikola::Vec3(0.0f);

  // Move forward
  if(nikola::input_key_down(nikola::KEY_W)) {
    velocity.x = PLAYER_SPEED;
  }
  // Move backwards
  else if(nikola::input_key_down(nikola::KEY_S)) {
    velocity.x = -PLAYER_SPEED;
  }
 
  // Move right
  if(nikola::input_key_down(nikola::KEY_A)) {
    velocity.z = -PLAYER_SPEED;
  }
  // Move left
  else if(nikola::input_key_down(nikola::KEY_D)) {
    velocity.z = PLAYER_SPEED;
  }
  
  nikola::physics_body_set_linear_velocity(player.body, velocity);
}

void player_set_active(Entity& player, const bool active) {
  player.is_active = active;
  nikola::physics_body_set_awake(player.body, active);
}

/// Player functions
/// ----------------------------------------------------------------------
