#pragma once

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// ResourceType
enum ResourceType {
  RESOURCE_CUBE = 0,
  RESOURCE_SKYBOX,

  RESOURCE_MATERIAL_PAVIMENT,
  RESOURCE_MATERIAL_ROAD,

  RESOURCE_CAR,
  RESOURCE_TRUCK,
  RESOURCE_COIN,
  RESOURCE_CONE,
  RESOURCE_TUNNEL,

  RESOURCE_SOUND_DEATH,
  RESOURCE_SOUND_KEY_COLLECT,
  RESOURCE_SOUND_WIN,
  RESOURCE_SOUND_FAIL_INPUT,
  
  RESOURCE_SOUND_UI_CLICK,
  RESOURCE_SOUND_UI_NAVIGATE,
  RESOURCE_SOUND_UI_TRANSITION,

  RESOURCE_SOUND_TILE_ROAD, 
  RESOURCE_SOUND_TILE_PAVIMENT, 

  RESOURCE_MUSIC_AMBIANCE,
  RESOURCE_MUSIC_HUB,

  RESOURCE_FONT,

  RESOURCES_MAX = RESOURCE_FONT + 1,
};
/// ResourceType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Resource database functions 

void resource_database_init();

void resource_database_shutdown();

const nikola::ResourceID& resource_database_get(const ResourceType type);

/// Resource database functions 
/// ----------------------------------------------------------------------
