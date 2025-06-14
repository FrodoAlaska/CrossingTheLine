#include "entity.h"
#include "levels/level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Consts

const float PLAYER_SPEED = 12.0f;

/// Consts
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
}

void player_update(Entity& player) {
  if(!player.is_active || !player.level_ref->main_camera.is_active) {
    return;
  }
  
  nikola::Vec3 velocity = nikola::Vec3(0.0f, -9.81f, 0.0f);

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

/// Player functions
/// ----------------------------------------------------------------------
