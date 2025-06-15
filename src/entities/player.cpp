#include "entity.h"
#include "levels/level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Consts

const float PLAYER_SPEED = 12.0f;

/// Consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Globals

static bool s_player_can_move = true;

/// Globals
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_raycast_hit(const nikola::Ray& ray, const nikola::RayIntersection& info, const nikola::Collider* coll) {
  Entity* entity = (Entity*)nikola::physics_body_get_user_data(nikola::collider_get_attached_body(coll));
  if(!info.has_intersected) {
    return;
  }
  
  s_player_can_move = (entity->type == ENTITY_TILE);
}

/// Callbacks
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

  // Raycast hit
  nikola::Ray ray = {
    .position  = nikola::physics_body_get_position(player.body),
    .direction = nikola::Vec3(0.0f, -1.0f, 0.0f),
  };
  nikola::physics_world_check_raycast(ray, on_raycast_hit);

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

/// Player functions
/// ----------------------------------------------------------------------
