#include "entity.h"
#include "levels/level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Tile functions

void tile_create(Tile* tile, Level* lvl, const TileType type, const nikola::Vec3& pos) {
  nikola::Vec3 scale    = nikola::Vec3(TILE_SIZE, 1.0f, TILE_SIZE);
  nikola::Vec3 offset   = nikola::Vec3(0.0f);
  nikola::Vec3 position = pos;

  // Different states depending on the tiles
  switch(type) {
    case TILE_CONE:
      scale      = nikola::Vec3(2.0f);
      position.y = -1.2f; 
      break;
    case TILE_TUNNEL_ONE_WAY:
      scale      = nikola::Vec3(1.5f, 2.0, 1.0f);
      offset     = nikola::Vec3(0.0f, 9.0f, 0.0f);
      position.y = 5.3f;
      break;
    case TILE_TUNNEL_TWO_WAY:
      scale      = nikola::Vec3(2.0f, 2.0f, 1.0f);
      offset     = nikola::Vec3(0.0f, 9.0f, 0.0f);
      position.y = 5.3f;
      break;
    case TILE_TUNNEL_THREE_WAY:
      scale      = nikola::Vec3(3.0f, 2.0f, 1.0f);
      offset     = nikola::Vec3(0.0f, 9.0f, 0.0f);
      position.y = 5.3f;
      break;
    default:
      break;
  }

  // Entity init
  entity_create(&tile->entity, 
                lvl, 
                position, 
                scale,
                ENTITY_TILE);
  
  // Tile variables init 
  tile->type = type; 
  
  // Collider position init
  nikola::collider_set_local_position(tile->entity.collider, offset);
}

/// Tile functions
/// ----------------------------------------------------------------------
