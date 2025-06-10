#include "entity.h"
#include "level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Player functions

void player_create(Entity* player, Level* lvl, const nikola::Vec3& start_pos) {
  // Entity init
  entity_create(player, 
                lvl, 
                start_pos, 
                nikola::Vec3(1.0f), 
                ENTITY_PLAYER, 
                nikola::PHYSICS_BODY_KINEMATIC);
}

void player_update(Entity& player) {
  if(!player.is_active) {
    return;
  }

  // Follow the camera
  nikola::physics_body_set_position(player.body, player.level_ref->main_camera.position);
}

/// Player functions
/// ----------------------------------------------------------------------
