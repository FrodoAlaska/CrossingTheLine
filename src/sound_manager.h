#pragma once

#include "levels/level.h"
#include "entities/entity.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// SoundType
enum SoundType {
  SOUND_DEATH, 
  SOUND_KEY_COLLECT,
  SOUND_WIN,

  SOUND_UI_CLICK, 
  SOUND_UI_NAVIGATE, 
  SOUND_UI_TRANSITION,

  SOUND_AMBIANCE,
  SOUND_HUB,

  SOUNDS_MAX = SOUND_HUB + 1,
};
/// SoundType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Sound manager functions 

void sound_manager_init(Level* lvl);

void sound_manager_shutdown();

/// Sound manager functions 
/// ----------------------------------------------------------------------
