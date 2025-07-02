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

const nikola::sizei POINTS_MAX       = 6;
const nikola::sizei TILES_MAX        = 256;
const nikola::sizei VEHICLES_MAX     = 12;
const nikola::sizei LEVEL_GROUPS_MAX = 5;

/// Consts
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

  nikola::Vec3 coin_position = nikola::Vec3(-1000.0f); 
  bool has_coin              = false;

  // End points
  
  struct NKEntity {
    nikola::Vec3 position; 
    nikola::Vec3 scale;
    nikola::u16 type;
  };

  nikola::sizei points_count = 0;
  NKEntity points[POINTS_MAX];

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

  // State

  nikola::Vec3 lerp_points[4];
  nikola::Vec3 current_lerp_point;
  nikola::Vec3 current_light_color;

  UIText pause_text;
  UILayout pause_layout;

  bool is_paused = false;
  
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
/// NKData functions

const bool nkdata_file_load(const nikola::FilePath& path);

void nkdata_file_save_current();

void nkdata_file_set_volume_data(const float master, const float music, const float sfx);

void nkdata_file_get_volume_data(float* master, float* music, float* sfx);

void nkdata_file_set_level_data(const nikola::u8 current_group, const nikola::u8 coins_collected);

void nkdata_file_get_level_data(nikola::u8* current_group, nikola::u8* coins_collected);

/// NKData functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Level functions

Level* level_create(nikola::Window* window);

bool level_load(Level* lvl, const nikola::FilePath& path);

void level_destroy(Level* lvl);

void level_unload(Level* lvl);

void level_reset(Level* lvl);

void level_process_input(Level* lvl);

void level_update(Level* lvl);

void level_render(Level* lvl);

void level_render_hud(Level* lvl);

void level_render_gui(Level* lvl);

/// Level functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Level manager functions

void level_manager_init(nikola::Window* window, const nikola::ResourceID& font_id);

void level_manager_shutdown();

void level_manager_reset();

void level_manager_advance();

void level_manager_process_input();

void level_manager_update(); 

void level_manager_render();

void level_manager_render_hud();

void level_manager_render_gui();

void level_manager_get_current_indices(nikola::sizei* group_index, nikola::sizei* level_index);

Level* level_manager_get_current_level();

/// Level manager functions
/// ----------------------------------------------------------------------
