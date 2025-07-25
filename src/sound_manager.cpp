#include "sound_manager.h"
#include "levels/level.h"
#include "entities/entity.h"
#include "states/state.h"
#include "resource_database.h"
#include "game_event.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// SoundManager
struct SoundManager {
  nikola::AudioSourceID entries[SOUNDS_MAX];
  nikola::sizei current_music = SOUND_AMBIANCE;
};

static SoundManager s_manager;
/// SoundManager
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_sound_play(const GameEvent& event, void* dispatcher, void* listener) {
  NIKOLA_ASSERT((event.sound_type >= 0 && event.sound_type <= SOUNDS_MAX), "Invalid SoundType given to event");
  
  switch(event.type) {
    case GAME_EVENT_SOUND_PLAYED:
      nikola::audio_source_start(s_manager.entries[event.sound_type]);
      break;
    case GAME_EVENT_MUSIC_PLAYED:
      nikola::audio_source_stop(s_manager.entries[s_manager.current_music]);
      nikola::audio_source_start(s_manager.entries[event.sound_type]);
      s_manager.current_music = event.sound_type;
      break;
    default:
      break;
  }
}

static void on_state_change(const GameEvent& event, void* dispatcher, void* listener) {
  switch(event.state_type) {
    case STATE_WON:
      nikola::audio_source_stop(s_manager.entries[SOUND_AMBIANCE]);
      nikola::audio_source_start(s_manager.entries[SOUND_WIN]);
      break;
    case STATE_LOST:
      nikola::audio_source_stop(s_manager.entries[SOUND_AMBIANCE]);
      nikola::audio_source_start(s_manager.entries[SOUND_DEATH]);
      break;
    default:
      break;
  }
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Sound manager functions 

void sound_manager_init() {
  // Extract volumes from NKData
  float master_volume = 1.0f; 
  float music_volume  = 1.0f;  
  float sfx_volume    = 1.0f;
  nkdata_file_get_volume_data(&master_volume, &master_volume, &sfx_volume);

  // Sound effects init
  for(nikola::sizei i = SOUND_DEATH; i <= SOUND_TILE_PAVIMENT; i++) {
    ResourceType res_type = (ResourceType)((nikola::sizei)(RESOURCE_SOUND_DEATH + i));

    nikola::AudioSourceDesc audio_desc; 
    audio_desc.volume        = sfx_volume; 
    audio_desc.buffers_count = 1; 
    audio_desc.buffers[0]    = nikola::resources_get_audio_buffer(resource_database_get(res_type));

    s_manager.entries[i] = nikola::audio_source_create(audio_desc); 
  }

  // Music init
  for(nikola::sizei i = SOUND_AMBIANCE; i < SOUNDS_MAX; i++) {
    ResourceType res_type = (ResourceType)((nikola::sizei)(RESOURCE_SOUND_DEATH + i));

    nikola::AudioSourceDesc audio_desc; 
    audio_desc.volume        = music_volume; 
    audio_desc.buffers_count = 1; 
    audio_desc.buffers[0]    = nikola::resources_get_audio_buffer(resource_database_get(res_type));

    s_manager.entries[i] = nikola::audio_source_create(audio_desc); 
  }

  // Audio listener init
  nikola::AudioListenerDesc listen_desc = {
    .volume = master_volume,
  };
  nikola::audio_listener_init(listen_desc); 

  // Listen to events
  game_event_listen(GAME_EVENT_SOUND_PLAYED, on_sound_play);
  game_event_listen(GAME_EVENT_MUSIC_PLAYED, on_sound_play);
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_change);

  NIKOLA_LOG_DEBUG("Initialized sound manager");
}

void sound_manager_shutdown() {
  // Sources destroy
  for(nikola::sizei i = 0; i < SOUNDS_MAX; i++) {
    nikola::audio_source_destroy(s_manager.entries[i]);
  }
}

void sound_manager_set_volume(const float master, const float music, const float sfx) {
  // Set master volume
  nikola::audio_listener_set_volume(master);

  // Set music volume
  for(nikola::sizei i = SOUND_AMBIANCE; i < SOUNDS_MAX; i++) {
    nikola::audio_source_set_volume(s_manager.entries[i], music);
  }

  // Set sound effects volume
  for(nikola::sizei i = SOUND_DEATH; i <= SOUND_TILE_PAVIMENT; i++) {
    nikola::audio_source_set_volume(s_manager.entries[i], sfx);
  }
}

/// Sound manager functions 
/// ----------------------------------------------------------------------
