#include "level.h"
#include "sound_manager.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// NKData
struct NKData {
  nikola::FilePath path;

  nikola::u8 current_group   = 0; 
  nikola::u8 coins_collected = 0;

  float master_volume = 1.0f; 
  float music_volume  = 1.0f; 
  float sfx_volume    = 1.0f;
};

static NKData s_data{};
/// NKData
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// NKData functions

const bool nkdata_file_load(const nikola::FilePath& path) {
  nikola::File file; 
  if(!nikola::file_open(&file, path, (int)(nikola::FILE_OPEN_READ | nikola::FILE_OPEN_BINARY))) {
    NIKOLA_LOG_ERROR("Failed to open NKData file at \'%s\'", path.c_str());

    s_data = {};
    return false;
  }

  // Path init
  s_data.path = path;

  // Load current group
  nikola::file_read_bytes(file, &s_data.current_group, sizeof(s_data.current_group));

  // Load coins collected
  nikola::file_read_bytes(file, &s_data.coins_collected, sizeof(s_data.coins_collected));

  // Load volume settings

  nikola::file_read_bytes(file, &s_data.master_volume, sizeof(s_data.master_volume));
  nikola::file_read_bytes(file, &s_data.music_volume, sizeof(s_data.music_volume));
  nikola::file_read_bytes(file, &s_data.sfx_volume, sizeof(s_data.sfx_volume));

  nikola::file_close(file);
}

void nkdata_file_save_current() {
  nikola::File file; 
  if(!nikola::file_open(&file, s_data.path, (int)(nikola::FILE_OPEN_WRITE | nikola::FILE_OPEN_BINARY))) {
    NIKOLA_LOG_ERROR("Failed to open NKData file at \'%s\'", s_data.path.c_str());
    return;
  }

  // Save current group
  nikola::file_write_bytes(file, &s_data.current_group, sizeof(s_data.current_group));

  // Save coins collected
  nikola::file_write_bytes(file, &s_data.coins_collected, sizeof(s_data.coins_collected));

  // Save volume settings

  nikola::file_write_bytes(file, &s_data.master_volume, sizeof(s_data.master_volume));
  nikola::file_write_bytes(file, &s_data.music_volume, sizeof(s_data.music_volume));
  nikola::file_write_bytes(file, &s_data.sfx_volume, sizeof(s_data.sfx_volume));
  
  nikola::file_close(file);
}

void nkdata_file_set_volume_data(const float master, const float music, const float sfx) {
  s_data.master_volume = master;
  s_data.music_volume  = music;
  s_data.sfx_volume    = sfx;

  nkdata_file_save_current();
  sound_manager_set_volume(master, music, sfx);
}

void nkdata_file_get_volume_data(float* master, float* music, float* sfx) {
  *master = s_data.master_volume;
  *music  = s_data.music_volume;
  *sfx    = s_data.sfx_volume;
}

void nkdata_file_set_level_data(const nikola::u8 current_group, const nikola::u8 coins_collected) {
  if(current_group > s_data.current_group) {
    s_data.current_group = current_group;
  }
  s_data.coins_collected = coins_collected;

  nkdata_file_save_current();
}

void nkdata_file_get_level_data(nikola::u8* current_group, nikola::u8* coins_collected) {
  *current_group   = s_data.current_group;
  *coins_collected = s_data.coins_collected;
}

/// NKData functions
/// ----------------------------------------------------------------------
