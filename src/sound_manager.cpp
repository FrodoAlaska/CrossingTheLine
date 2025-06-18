#include "sound_manager.h"
#include "levels/level.h"
#include "entities/entity.h"
#include "game_event.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// SoundManager
struct SoundManager {
  Level* level_ref;
  nikola::AudioSourceID entries[SOUNDS_MAX];
};

static SoundManager s_manager;
/// SoundManager
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static bool on_sound_play(const GameEvent& event, void* dispatcher, void* listener) {
  NIKOLA_ASSERT((event.sound_type >= 0 && event.sound_type <= SOUNDS_MAX), "Invalid SoundType given to event");

  switch(event.type) {
    case GAME_EVENT_SOUND_PLAYED:
      nikola::audio_source_start(s_manager.entries[event.sound_type]);
      return true;
    case GAME_EVENT_LEVEL_WON:
      nikola::audio_source_stop(s_manager.entries[SOUND_AMBIANCE]);
      nikola::audio_source_start(s_manager.entries[SOUND_WIN]);
      return true;
    case GAME_EVENT_LEVEL_LOST:
      nikola::audio_source_stop(s_manager.entries[SOUND_AMBIANCE]);
      nikola::audio_source_start(s_manager.entries[SOUND_DEATH]);
      return true;
    case GAME_EVENT_COIN_COLLECTED:
      nikola::audio_source_start(s_manager.entries[SOUND_KEY_COLLECT]);
      return true;
    default:
      return false;
  }

  return true;
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Sound manager functions 

void sound_manager_init(Level* lvl) {
  // Level ref init
  s_manager.level_ref = lvl;

  // Audio sources init
  for(nikola::sizei i = 0; i < SOUNDS_MAX; i++) {
    nikola::AudioSourceDesc audio_desc; 
    audio_desc.volume        = 0.4f; 
    audio_desc.buffers_count = 1; 
    audio_desc.buffers[0]    = nikola::resources_get_audio_buffer(lvl->resources[LEVEL_RESOURCE_SOUND_DEATH + i]);

    s_manager.entries[i] = nikola::audio_source_create(audio_desc); 
  }

  // Audio listener init
  nikola::AudioListenerDesc listen_desc = {};
  nikola::audio_listener_init(listen_desc); 

  // Listen to events
  game_event_listen(GAME_EVENT_SOUND_PLAYED, on_sound_play);
  game_event_listen(GAME_EVENT_LEVEL_WON, on_sound_play);
  game_event_listen(GAME_EVENT_LEVEL_LOST, on_sound_play);
  game_event_listen(GAME_EVENT_COIN_COLLECTED, on_sound_play);

  NIKOLA_LOG_TRACE("Initialized sound manager");
}

void sound_manager_shutdown() {
  // Sources destroy
  for(nikola::sizei i = 0; i < SOUNDS_MAX; i++) {
    nikola::audio_source_destroy(s_manager.entries[i]);
  }
}

/// Sound manager functions 
/// ----------------------------------------------------------------------
