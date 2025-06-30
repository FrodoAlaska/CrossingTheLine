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
    case TILE_CONE:
      scale      = nikola::Vec3(2.0f);
      position.y = -1.2f; 
      break;
    case TILE_TUNNEL_ONE_WAY:
      scale = nikola::Vec3(0.015f);
      break;
    case TILE_TUNNEL_TWO_WAY:
      scale = nikola::Vec3(0.02f, 0.015f, 0.015f);
      break;
    case TILE_TUNNEL_THREE_WAY:
      scale = nikola::Vec3(0.03f, 0.015f, 0.015f);
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
}

/// Tile functions
/// ----------------------------------------------------------------------
