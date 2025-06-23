#include "entity.h"
#include "levels/level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Tile functions

void tile_create(Tile* tile, Level* lvl, const TileType type, const nikola::Vec3& pos) {
  nikola::Vec3 scale = nikola::Vec3(TILE_SIZE, 1.0f, TILE_SIZE);

  // Different states depending on the tiles
  switch(type) {
    case TILE_CONE:
      scale = nikola::Vec3(2.0f);
      break;
    default:
      break;
  }

  // Entity init
  entity_create(&tile->entity, 
                lvl, 
                pos, 
                scale,
                ENTITY_TILE);
  
  // Tile variables init 
  tile->type = type; 
}

/// Tile functions
/// ----------------------------------------------------------------------
