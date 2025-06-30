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
  // Entity init
  entity_create(&player->entity, 
                lvl, 
                start_pos, 
                nikola::Vec3(1.2f, 3.0f, 1.2f),
                ENTITY_PLAYER, 
                nikola::PHYSICS_BODY_KINEMATIC);
  
  // Player variables init
  player->current_footstep_sound = SOUND_TILE_PAVIMENT;
  player->can_move               = true;
}

void player_update(Player& player) {
  if(!player.entity.is_active) {
    return;
  }

  // Movement
 
  // Apply some gravity if the player is currently not allowed to move
  if(!player.can_move) {
    nikola::Vec3 current_velocity = nikola::physics_body_get_linear_velocity(player.entity.body);
    nikola::physics_body_set_linear_velocity(player.entity.body, nikola::Vec3(current_velocity.x, -9.81f, current_velocity.z));

    return;
  }

  // Apply the velocity
  nikola::Vec3 velocity = input_manager_get_movement_velocity() * PLAYER_SPEED;
  nikola::physics_body_set_linear_velocity(player.entity.body, velocity);

  // Play the footstep sound if possible
  if(velocity.x != 0 || velocity.z != 0) {
    game_event_dispatch(GameEvent{
      .type       = GAME_EVENT_SOUND_PLAYED, 
      .sound_type = player.current_footstep_sound,
    });
  }
 
  // Make the camera follow the player's X position. 
  //
  // @TODO: Perhaps using something better than lerp for a smoother transition?
  
  nikola::Camera* camera = &player.entity.level_ref->main_camera;
  nikola::Vec3 position  = nikola::physics_body_get_position(player.entity.body);
  
  camera->position.x = nikola::lerp(camera->position.x, position.x - 20.0f, nikola::niclock_get_delta_time() * 2.0f);
}

void player_set_active(Player& player, const bool active) {
  player.entity.is_active = active;
  nikola::physics_body_set_awake(player.entity.body, active);
  nikola::physics_body_set_linear_velocity(player.entity.body, nikola::Vec3(0.0f));
}

/// Player functions
/// ----------------------------------------------------------------------
