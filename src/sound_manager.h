#pragma once

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// SoundType
enum SoundType {
  SOUND_DEATH, 
  SOUND_KEY_COLLECT,
  SOUND_WIN,
  SOUND_FAIL_INPUT,

  SOUND_UI_CLICK, 
  SOUND_UI_NAVIGATE, 
  SOUND_UI_TRANSITION,
  
  SOUND_TILE_ROAD, 
  SOUND_TILE_PAVIMENT,

  SOUND_AMBIANCE,
  SOUND_HUB,
  SOUND_MUSIC_WON,

  SOUNDS_MAX = SOUND_MUSIC_WON + 1,
};
/// SoundType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Sound manager functions 

void sound_manager_init();

void sound_manager_shutdown();

/// Sound manager functions 
/// ----------------------------------------------------------------------
