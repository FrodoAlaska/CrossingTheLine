#pragma once

#include "entities\entity.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// LevelResourceType
enum LevelResourceType {
  LEVEL_RESOURCE_CUBE = 0, 

  LEVEL_RESOURCE_MATERIAL_PAVIMENT, 
  LEVEL_RESOURCE_MATERIAL_ROAD, 

  LEVEL_RESOURCE_CAR, 
  LEVEL_RESOURCE_TRUCK,
  
  LEVEL_RESOURCE_FONT,

  LEVEL_RESOURCES_MAX = LEVEL_RESOURCE_FONT + 1,
};
/// LevelResourceType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Level
struct Level {
  nikola::FilePath path;
  nikola::Window* window_ref; 

  // Renderables

  nikola::Camera main_camera; 
  nikola::Camera gui_camera;
  nikola::Camera* current_camera;
  nikola::FrameData frame; 
  
  // Resources

  nikola::ResourceGroupID resource_group;
  nikola::ResourceID resources[LEVEL_RESOURCES_MAX];

  // Entities

  Entity player; 
  nikola::DynamicArray<Entity> end_points;
  nikola::DynamicArray<Vehicle> vehicles;
  nikola::DynamicArray<Tile> tiles;

  // State

  bool has_lost  = false;
  bool has_won   = false;
  bool is_paused = false;
  
  // Debug stuff
  
  bool has_editor = false;
  bool debug_mode = false;

  Entity debug_plane;
  nikola::Vec3 debug_selection = nikola::Vec3(10.0f, -3.0f, 10.0f);
};
/// Level
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Level functions

Level* level_create(nikola::Window* window);

bool level_load(Level* lvl, const nikola::FilePath& path);

void level_destroy(Level* lvl);

void level_unload(Level* lvl);

void level_update(Level* lvl);

void level_render(Level* lvl);

void level_render_hud(Level* lvl);

void level_render_gui(Level* lvl);

/// Level functions
/// ----------------------------------------------------------------------
