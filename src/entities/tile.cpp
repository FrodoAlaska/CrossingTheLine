#include "entity.h"
#include "levels/level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Tile functions

void tile_create(Tile* tile, Level* lvl, const TileType type, const nikola::Vec3& pos) {
  nikola::Vec3 scale    = nikola::Vec3(TILE_SIZE, 1.0f, TILE_SIZE);
  nikola::Vec3 position = pos;

  // Different states depending on the tiles
  switch(type) {
    case TILE_PAVIMENT:
      position.y = -1.95f; 
      break;
    case TILE_CONE:
      scale      = nikola::Vec3(2.0f);
      position.y = -1.2f; 
      break;
    case TILE_TUNNEL_ONE_WAY:
      scale      = nikola::Vec3(18.0f, 10.0f, 24.0f);
      position.y = 5.3f;
      break;
    case TILE_TUNNEL_TWO_WAY:
      scale      = nikola::Vec3(24.0f, 10.0f, 24.0f);
      position.y = 5.3f;
      break;
    case TILE_TUNNEL_THREE_WAY:
      scale      = nikola::Vec3(30.0f, 10.0f, 24.0f);
      position.y = 5.3f;
      break;
    default:
      break;
  }

  // Entity variables init
  tile->entity.type      = ENTITY_TILE;
  tile->entity.level_ref = lvl;
  tile->entity.is_active = true;
  tile->entity.start_pos = position;
  
  // Tile variables init 
  tile->type = type; 

  // Body init
  nikola::PhysicsBodyDesc body_desc = {
    .position  = tile->entity.start_pos, 
    .type      = nikola::PHYSICS_BODY_STATIC,
    .layers    = PHYSICS_LAYER_1,
    .user_data = &tile->entity,
  };
  tile->entity.body = nikola::physics_body_create(body_desc);

  // Collider init
  nikola::ColliderDesc coll_desc = {
    .position  = nikola::Vec3(0.0f),
    .extents   = scale,
    .friction  = 0.0f,
    .is_sensor = false,
  };
  tile->entity.collider = nikola::physics_body_add_collider(tile->entity.body, coll_desc);
}

/// Tile functions
/// ----------------------------------------------------------------------
