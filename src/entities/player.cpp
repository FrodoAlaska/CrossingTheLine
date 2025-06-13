#include "entity.h"
#include "levels/level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Player functions

void player_create(Entity* player, Level* lvl, const nikola::Vec3& start_pos) {
  // Entity init
  entity_create(player, 
                lvl, 
                start_pos, 
                nikola::Vec3(1.0f, 4.0f, 1.0f), 
                ENTITY_PLAYER, 
                nikola::PHYSICS_BODY_KINEMATIC);
}

void player_update(Entity& player) {
  if(!player.is_active || !player.level_ref->main_camera.is_active) {
    return;
  }

  float speed           = 10.0f;
  nikola::Camera camera = player.level_ref->main_camera; 
  nikola::physics_body_set_linear_velocity(player.body, nikola::Vec3(0.0f));

  // Move forward
  if(nikola::input_key_down(nikola::KEY_W)) {
    nikola::Vec3 force = nikola::Vec3(camera.front.x, 0.0f, camera.front.z) * speed;
    nikola::physics_body_set_linear_velocity(player.body, force);
  }
  // Move backwards
  else if(nikola::input_key_down(nikola::KEY_S)) {
    nikola::Vec3 force = nikola::Vec3(camera.front.x, 0.0f, camera.front.z) * speed;
    nikola::physics_body_set_linear_velocity(player.body, -force);
  }
 
  // Move right
  if(nikola::input_key_down(nikola::KEY_A)) {
    nikola::Vec3 force = nikola::vec3_normalize(nikola::vec3_cross(camera.front, camera.up)) * speed;
    nikola::physics_body_set_linear_velocity(player.body, -force);
  }
  // Move left
  else if(nikola::input_key_down(nikola::KEY_D)) {
    nikola::Vec3 force = nikola::vec3_normalize(nikola::vec3_cross(camera.front, camera.up)) * speed;
    nikola::physics_body_set_linear_velocity(player.body, force);
  }

  // Follow the camera
  player.level_ref->main_camera.position = nikola::physics_body_get_position(player.body);
}

/// Player functions
/// ----------------------------------------------------------------------
