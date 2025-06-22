#pragma once

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------

// Much needed forward declarations
struct Level;

/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Consts

const float TILE_SIZE = 8.0f;

/// Consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// EntityType 
enum EntityType {
  ENTITY_PLAYER      = 0x20, 
  ENTITY_VEHICLE     = 0x40, 
  ENTITY_TILE        = 0x60,

  ENTITY_END_POINT     = 0x80,
  ENTITY_DEATH_POINT   = 0x100,
  ENTITY_VEHICLE_POINT = 0x120,
  ENTITY_CHAPTER_POINT = 0x140,
  
  ENTITY_COIN        = 0x160,
};
/// EntityType 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// VehicleType
enum VehicleType {
  VEHICLE_CAR,
  VEHICLE_TRUCK,
};
/// VehicleType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// TileType 
enum TileType {
  TILE_ROAD, 
  TILE_PAVIMENT,
  TILE_TUNNEL,
};
/// TileType 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Entity 
struct Entity {
  EntityType type;
  Level* level_ref;
  bool is_active;

  nikola::Vec3 start_pos;
  nikola::PhysicsBody* body; 
  nikola::Collider* collider; 
};
/// Entity 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Player 
struct Player {
  Entity entity; 

  int current_footstep_sound = -1;
  bool can_move              = true;
};
/// Player 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Vehicle 
struct Vehicle {
  Entity entity;

  VehicleType type;
  float acceleration; 

  nikola::Vec3 direction;
  nikola::Vec4 rotation;
};
/// Vehicle 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Tile 
struct Tile {
  Entity entity;
  TileType type; 
};
/// Tile 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Generic entity functions

void entity_create(Entity* entity, 
                   Level* lvl, 
                   const nikola::Vec3& pos, 
                   const nikola::Vec3& scale, 
                   const EntityType entt_type, 
                   const nikola::PhysicsBodyType body_type = nikola::PHYSICS_BODY_STATIC, 
                   const bool is_sensor = false);

const bool entity_aabb_test(Entity& entity, Entity& other);

/// Generic entity functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Player functions

void player_create(Player* player, Level* lvl, const nikola::Vec3& position);

void player_update(Player& player); 

void player_set_active(Player& player, const bool active);

/// Player functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Vehicle functions

void vehicle_create(Vehicle* v,  
                    Level* lvl, 
                    const VehicleType type, 
                    const nikola::Vec3& position, 
                    const nikola::Vec3& dir, 
                    const float acceleration = 30.0f);

void vehicle_set_active(Vehicle& v, const bool active);

/// Vehicle functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Tile functions

void tile_create(Tile* tile, Level* lvl, const TileType type, const nikola::Vec3& pos);

/// Tile functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Entity manager functions

void entity_manager_create(Level* level_ref);

void entity_manager_destroy();

void entity_manager_load();

void entity_manager_save();

void entity_manager_reset();

void entity_manager_update();

void entity_manager_render();

void entity_manager_render_gui();

/// Entity manager functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Tile manager functions

void tile_manager_create(Level* level_ref);

void tile_manager_destroy();

void tile_manager_load(); 

void tile_manager_save();

void tile_manager_update();

void tile_manager_render();

void tile_manager_render_gui();

void tile_manager_check_collisions(Player& player);

/// Tile manager functions
/// ----------------------------------------------------------------------
