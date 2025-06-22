#include "entity.h"
#include "levels/level.h"
#include "sound_manager.h"
#include "game_event.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Consts

const float PLAYER_SPEED = 9.5f;

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
    nikola::physics_body_set_linear_velocity(player.entity.body, nikola::Vec3(0.0f, -9.81f, 0.0f));
    return;
  }

  nikola::Vec3 velocity = nikola::Vec3(0.0f);
  bool can_play_sound   = false;

  // Move forward
  if(nikola::input_key_down(nikola::KEY_W)) {
    velocity.x     = PLAYER_SPEED;
    can_play_sound = true;
  }
  // Move backwards
  else if(nikola::input_key_down(nikola::KEY_S)) {
    velocity.x     = -PLAYER_SPEED;
    can_play_sound = true;
  }
 
  // Move right
  if(nikola::input_key_down(nikola::KEY_A)) {
    velocity.z     = -PLAYER_SPEED;
    can_play_sound = true;
  }
  // Move left
  else if(nikola::input_key_down(nikola::KEY_D)) {
    velocity.z     = PLAYER_SPEED;
    can_play_sound = true;
  }

  // Apply the velocity
  nikola::physics_body_set_linear_velocity(player.entity.body, velocity);

  // Play the footstep sound if possible
  if(can_play_sound) {
    game_event_dispatch(GameEvent{
      .type       = GAME_EVENT_SOUND_PLAYED, 
      .sound_type = player.current_footstep_sound,
    });
  }
}

void player_set_active(Player& player, const bool active) {
  player.entity.is_active = active;
  nikola::physics_body_set_awake(player.entity.body, active);
}

/// Player functions
/// ----------------------------------------------------------------------
