#pragma once

#include "entities\entity.h"
#include "ui\ui.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Consts

/// @NOTE: These constants are not the final required sizes. They are 
/// only here to avoid memory allocation when loading a `nklvl` file. 
/// Besides that, they help to set light boundries to the amount of 
/// entities we can have in a level. Again, though, they are not "set in stone", per se. 

const nikola::sizei END_POINTS_MAX = 6;
const nikola::sizei TILES_MAX      = 256;
const nikola::sizei VEHICLES_MAX   = 12;

/// Consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// LevelResourceType
enum LevelResourceType {
  LEVEL_RESOURCE_CUBE = 0,

  LEVEL_RESOURCE_MATERIAL_PAVIMENT = 1,
  LEVEL_RESOURCE_MATERIAL_ROAD     = 2,

  LEVEL_RESOURCE_CAR   = 3,
  LEVEL_RESOURCE_TRUCK = 4,
  LEVEL_RESOURCE_COIN  = 5,

  LEVEL_RESOURCE_FONT = 6,

  LEVEL_RESOURCES_MAX = LEVEL_RESOURCE_FONT + 1,
};
/// LevelResourceType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// NKLevelFile
struct NKLevelFile {
  nikola::FilePath path;

  // Versions

  nikola::u8 major_version, minor_version;
  
  // Player 

  nikola::Vec3 start_position;

  // Coin

  nikola::Vec3 coin_position; 
  bool has_coin = false;

  // End points
  
  struct NKEntity {
    nikola::Vec3 position; 
    nikola::Vec3 scale;
  };

  nikola::sizei end_points_count = 0;
  NKEntity end_points[END_POINTS_MAX];

  // Vehicles

  struct NKVehicle {
    nikola::Vec3 position;
    nikola::Vec3 direction; 

    float acceleration;
    nikola::u8 vehicle_type;
  };

  nikola::sizei vehicles_count = 0;
  NKVehicle vehicles[VEHICLES_MAX];

  // Tiles

  struct NKTile {
    nikola::Vec3 position;
    nikola::u8 tile_type;
  };

  nikola::sizei tiles_count = 0; 
  NKTile tiles[TILES_MAX];
};
/// NKLevelFile
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Level
struct Level {
  NKLevelFile nkbin;
  nikola::Window* window_ref; 

  // Renderables

  nikola::Camera main_camera; 
  nikola::Camera gui_camera;
  nikola::Camera* current_camera;
  nikola::FrameData frame; 
  
  // Resources

  nikola::ResourceGroupID resource_group;
  nikola::ResourceID resources[LEVEL_RESOURCES_MAX];

  // State

  bool is_paused = false;
  bool has_coin  = true;

  // Debug stuff
  
  bool has_editor = false;
  bool debug_mode = false;
};
/// Level
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// NKLevelFile functions

const bool nklvl_file_load(NKLevelFile* nklvl, const nikola::FilePath& path);

void nklvl_file_save(const NKLevelFile& nklvl);

/// NKLevelFile functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Level functions

Level* level_create(nikola::Window* window);

bool level_load(Level* lvl, const nikola::FilePath& path);

void level_destroy(Level* lvl);

void level_unload(Level* lvl);

void level_reset(Level* lvl);

void level_update(Level* lvl);

void level_render(Level* lvl);

void level_render_hud(Level* lvl);

void level_render_gui(Level* lvl);

/// Level functions
/// ----------------------------------------------------------------------
