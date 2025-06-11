#include "entity.h"
#include "levels/level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Tile functions

void tile_create(Tile* tile, Level* lvl, const TileType type, const nikola::Vec3& pos) {
  // Entity init
  entity_create(&tile->entity, 
                lvl, 
                pos, 
                nikola::Vec3(TILE_SIZE, 1.0f, TILE_SIZE),
                ENTITY_TILE);
  
  // Tile variables init 
  tile->type = type; 
}

/// Tile functions
/// ----------------------------------------------------------------------
