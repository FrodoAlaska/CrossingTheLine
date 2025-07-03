#include "entity.h"
#include "levels/level.h"
#include "sound_manager.h"
#include "input_manager.h"
#include "game_event.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Consts

const float PLAYER_SPEED = 11.2f;

/// Consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Player functions

void player_create(Player* player, Level* lvl, const nikola::Vec3& start_pos) {
  // Entity variables init
  player->entity.type      = ENTITY_PLAYER;
  player->entity.level_ref = lvl;
  player->entity.is_active = true;
  player->entity.start_pos = start_pos;
  
  // Player variables init
  player->current_footstep_sound = SOUND_TILE_PAVIMENT;
  player->can_move               = true;

  // Body init
  nikola::PhysicsBodyDesc body_desc = {
    .position      = player->entity.start_pos, 
    .type          = nikola::PHYSICS_BODY_DYNAMIC,
    .locked_axises = nikola::BVec3(true),
    .layers        = (PHYSICS_LAYER_0 | PHYSICS_LAYER_1),
    .user_data     = &player->entity,
  };
  player->entity.body = nikola::physics_body_create(body_desc);

  // Collider init
  nikola::ColliderDesc coll_desc = {
    .position  = nikola::Vec3(0.0f), 
    .extents   = nikola::Vec3(1.2f, 3.4f, 1.2f),
    .friction  = 0.0f,
    .is_sensor = false,
  };
  player->entity.collider = nikola::physics_body_add_collider(player->entity.body, coll_desc);
}

void player_update(Player& player) {
  if(!player.entity.is_active) {
    return;
  }

  // Movement

  // @TODO: Apply some gravity if the player is currently not allowed to move
  if(!player.can_move) {
    nikola::physics_body_apply_force(player.entity.body, nikola::Vec3(0.0f, -9.81f, 0.0f));
  }

  // Apply the velocity
  nikola::Vec3 velocity = input_manager_get_movement_velocity() * PLAYER_SPEED;
  nikola::Vec3 current_velocity = nikola::physics_body_get_linear_velocity(player.entity.body);

  nikola::physics_body_set_linear_velocity(player.entity.body, 
                                           nikola::Vec3(velocity.x, current_velocity.y, velocity.z));

  // Make the camera follow the player's X position. 
  //
  // @TODO: Perhaps using something better than lerp for a smoother transition?
  
  nikola::Camera* camera = &player.entity.level_ref->main_camera;
  nikola::Vec3 position  = nikola::physics_body_get_position(player.entity.body);
  
  camera->position.x = nikola::lerp(camera->position.x, position.x - 20.0f, nikola::niclock_get_delta_time() * 2.0f);

  position.x = nikola::clamp_float(position.x, -27.5f, 100.0f);
  nikola::physics_body_set_position(player.entity.body, position);
}

void player_set_active(Player& player, const bool active) {
  player.entity.is_active = active;
  nikola::physics_body_set_awake(player.entity.body, active);
  nikola::physics_body_set_linear_velocity(player.entity.body, nikola::Vec3(0.0f));
}

/// Player functions
/// ----------------------------------------------------------------------
