#include "level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Consts

const nikola::u8 NKLVL_VERSION_MAJOR = 0; 
const nikola::u8 NKLVL_VERSION_MINOR = 2; 

/// Consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// NKLevelFile functions

const bool nklvl_file_load(NKLevelFile* nklvl, const nikola::FilePath& path) {
  // Path init (to save the file if needed later)
  nklvl->path = path;

  // Open the file first
  nikola::File file; 
  if(!nikola::file_open(&file, nklvl->path, (int)(nikola::FILE_OPEN_READ | nikola::FILE_OPEN_BINARY))) {
    NIKOLA_LOG_ERROR("Failed to read the level file at \'%s\'", nklvl->path.c_str());
    return false;
  }

  // Read the versions
  nikola::u8 major_version, minor_version; 
  nikola::file_read_bytes(file, &nklvl->major_version, sizeof(nklvl->major_version));
  nikola::file_read_bytes(file, &nklvl->minor_version, sizeof(nklvl->minor_version));

  // Checking for the file's validity
  bool is_valid = (nklvl->major_version == NKLVL_VERSION_MAJOR) && (nklvl->minor_version == NKLVL_VERSION_MINOR);
  NIKOLA_ASSERT(is_valid, "Found invalid level binary version in given path");

  // Read the starting position 
  nikola::file_read_bytes(file, &nklvl->start_position[0], sizeof(nklvl->start_position));

  // Read the coin
  
  nikola::file_read_bytes(file, &nklvl->coin_position[0], sizeof(nklvl->coin_position));
  nikola::file_read_bytes(file, &nklvl->has_coin, sizeof(nklvl->has_coin));

  // Read the end points
  
  nikola::file_read_bytes(file, &nklvl->end_points_count, sizeof(nklvl->end_points_count));
  for(nikola::sizei i = 0; i < nklvl->end_points_count; i++) {
    nikola::file_read_bytes(file, &nklvl->end_points[i].position[0], sizeof(nikola::Vec3));
    nikola::file_read_bytes(file, &nklvl->end_points[i].scale[0], sizeof(nikola::Vec3));
  }

  // Read the vehicles

  nikola::file_read_bytes(file, &nklvl->vehicles_count, sizeof(nklvl->vehicles_count));
  for(nikola::sizei i = 0; i < nklvl->vehicles_count; i++) {
    nikola::file_read_bytes(file, &nklvl->vehicles[i].position[0], sizeof(nikola::Vec3));
    nikola::file_read_bytes(file, &nklvl->vehicles[i].direction[0], sizeof(nikola::Vec3));
    nikola::file_read_bytes(file, &nklvl->vehicles[i].acceleration, sizeof(float));
    nikola::file_read_bytes(file, &nklvl->vehicles[i].vehicle_type, sizeof(nikola::u8));
  }

  // Read the tiles
  
  nikola::file_read_bytes(file, &nklvl->tiles_count, sizeof(nklvl->tiles_count));
  for(nikola::sizei i = 0; i < nklvl->tiles_count; i++) {
    nikola::file_read_bytes(file, &nklvl->tiles[i].position[0], sizeof(nikola::Vec3));
    nikola::file_read_bytes(file, &nklvl->tiles[i].tile_type, sizeof(nikola::u8));
  }

  // Always remember to close the file
  nikola::file_close(file);
  return true;
}

void nklvl_file_save(const NKLevelFile& nklvl) {
  // Open the file first
  nikola::File file; 
  if(!nikola::file_open(&file, nklvl.path, (int)(nikola::FILE_OPEN_WRITE | nikola::FILE_OPEN_BINARY))) {
    NIKOLA_LOG_ERROR("Failed to save the level file at \'%s\'", nklvl.path.c_str());
    return;
  }

  // Wrtie the versions
  nikola::file_write_bytes(file, &nklvl.major_version, sizeof(nklvl.major_version));
  nikola::file_write_bytes(file, &nklvl.minor_version, sizeof(nklvl.minor_version));

  // Wrtie the starting position 
  nikola::file_write_bytes(file, &nklvl.start_position[0], sizeof(nklvl.start_position));
  
  // Write the coin
  
  nikola::file_write_bytes(file, &nklvl.coin_position[0], sizeof(nklvl.coin_position));
  nikola::file_write_bytes(file, &nklvl.has_coin, sizeof(nklvl.has_coin));

  // Wrtie the end points
  
  nikola::file_write_bytes(file, &nklvl.end_points_count, sizeof(nklvl.end_points_count));

  for(nikola::sizei i = 0; i < nklvl.end_points_count; i++) {
    nikola::file_write_bytes(file, &nklvl.end_points[i].position[0], sizeof(nikola::Vec3));
    nikola::file_write_bytes(file, &nklvl.end_points[i].scale[0], sizeof(nikola::Vec3));
  }

  // Wrtie the vehicles

  nikola::file_write_bytes(file, &nklvl.vehicles_count, sizeof(nklvl.vehicles_count));

  for(nikola::sizei i = 0; i < nklvl.vehicles_count; i++) {
    nikola::Vec3 position   = nklvl.vehicles[i].position;
    nikola::u8 vehicle_type = (nikola::u8)nklvl.vehicles[i].vehicle_type;

    nikola::file_write_bytes(file, &nklvl.vehicles[i].position[0], sizeof(nikola::Vec3));
    nikola::file_write_bytes(file, &nklvl.vehicles[i].direction[0], sizeof(nikola::Vec3));
    nikola::file_write_bytes(file, &nklvl.vehicles[i].acceleration, sizeof(float));
    nikola::file_write_bytes(file, &nklvl.vehicles[i].vehicle_type, sizeof(nikola::u8));
  }

  // Write the tiles 

  nikola::file_write_bytes(file, &nklvl.tiles_count, sizeof(nklvl.tiles_count));

  for(nikola::sizei i = 0; i < nklvl.tiles_count; i++) {
    nikola::file_write_bytes(file, &nklvl.tiles[i].position[0], sizeof(nikola::Vec3));
    nikola::file_write_bytes(file, &nklvl.tiles[i].tile_type, sizeof(nikola::u8));
  }

  // Always remember to close the file
  nikola::file_close(file);
  NIKOLA_LOG_TRACE("Saved level file at \'%s\'", nklvl.path.c_str());
}

/// NKLevelFile functions
/// ----------------------------------------------------------------------
