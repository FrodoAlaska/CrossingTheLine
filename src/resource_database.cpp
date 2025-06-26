#include "resource_database.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// ResourceDatabase
struct ResourceDatabase {
  nikola::ResourceGroupID resource_group;
  nikola::ResourceID resources[RESOURCES_MAX];
};

static ResourceDatabase s_database;
/// ResourceDatabase
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Resource database functions 

void resource_database_init() {
  // Resource group init
  s_database.resource_group = nikola::resources_create_group("level_res", nikola::filesystem_current_path());

  // Resources init
  nikola::resources_push_dir(s_database.resource_group, "res");

  // Meshes init
  s_database.resources[RESOURCE_CUBE] = nikola::resources_push_mesh(s_database.resource_group, nikola::GEOMETRY_CUBE);
  
  // Skybox init
  nikola::ResourceID cubemap_id         = nikola::resources_get_id(s_database.resource_group, "dreamy_sky");
  s_database.resources[RESOURCE_SKYBOX] = nikola::resources_push_skybox(s_database.resource_group, cubemap_id);

  // Materials init
  
  s_database.resources[RESOURCE_MATERIAL_PAVIMENT] = nikola::resources_push_material(s_database.resource_group, nikola::resources_get_id(s_database.resource_group, "paviment"));
  s_database.resources[RESOURCE_MATERIAL_ROAD]     = nikola::resources_push_material(s_database.resource_group, nikola::resources_get_id(s_database.resource_group, "road"));

  // Models init

  s_database.resources[RESOURCE_CAR]   = nikola::resources_get_id(s_database.resource_group, "sedan");
  s_database.resources[RESOURCE_TRUCK] = nikola::resources_get_id(s_database.resource_group, "delivery");
  s_database.resources[RESOURCE_COIN]  = nikola::resources_get_id(s_database.resource_group, "gold_key");
  s_database.resources[RESOURCE_CONE]  = nikola::resources_get_id(s_database.resource_group, "cone");
 
  // Sounds init
 
  s_database.resources[RESOURCE_SOUND_DEATH]       = nikola::resources_get_id(s_database.resource_group, "sfx_death");
  s_database.resources[RESOURCE_SOUND_KEY_COLLECT] = nikola::resources_get_id(s_database.resource_group, "sfx_key_collect");
  s_database.resources[RESOURCE_SOUND_WIN]         = nikola::resources_get_id(s_database.resource_group, "sfx_win");
  s_database.resources[RESOURCE_SOUND_FAIL_INPUT]  = nikola::resources_get_id(s_database.resource_group, "sfx_fail_input");
  
  s_database.resources[RESOURCE_SOUND_UI_CLICK]      = nikola::resources_get_id(s_database.resource_group, "sfx_ui_click");
  s_database.resources[RESOURCE_SOUND_UI_NAVIGATE]   = nikola::resources_get_id(s_database.resource_group, "sfx_ui_navigate");
  s_database.resources[RESOURCE_SOUND_UI_TRANSITION] = nikola::resources_get_id(s_database.resource_group, "sfx_transition");
  
  s_database.resources[RESOURCE_SOUND_TILE_ROAD]     = nikola::resources_get_id(s_database.resource_group, "sfx_road");
  s_database.resources[RESOURCE_SOUND_TILE_PAVIMENT] = nikola::resources_get_id(s_database.resource_group, "sfx_paviment");
  
  s_database.resources[RESOURCE_MUSIC_AMBIANCE] = nikola::resources_get_id(s_database.resource_group, "music_ambiance");
  s_database.resources[RESOURCE_MUSIC_HUB]      = nikola::resources_get_id(s_database.resource_group, "music_nocturne");

  // Font init 
  s_database.resources[RESOURCE_FONT] = nikola::resources_get_id(s_database.resource_group, "iosevka_bold");
}

void resource_database_shutdown() {
  nikola::resources_destroy_group(s_database.resource_group);
}

const nikola::ResourceID& resource_database_get(const ResourceType type) {
  return s_database.resources[type];
}

/// Resource database functions 
/// ----------------------------------------------------------------------
