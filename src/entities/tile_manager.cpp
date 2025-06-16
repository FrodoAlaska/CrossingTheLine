#include "entity.h"
#include "levels/level.h"

#include <nikola/nikola.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

/// ----------------------------------------------------------------------
/// TileManager
struct TileManager {
  Level* level_ref;

  nikola::DynamicArray<Tile> tiles;
  nikola::Vec3 debug_selection;
};

static TileManager s_tiles;
/// TileManager
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Tile manager functions

void tile_manager_create(Level* level_ref) {
  // Level init
  s_tiles.level_ref = level_ref;

  // Debug selection init
  s_tiles.debug_selection = nikola::Vec3(TILE_SIZE, -2.0f, TILE_SIZE);
}

void tile_manager_destroy() {
  // Tiles destroy
  for(auto& tile : s_tiles.tiles) {
    nikola::physics_body_destroy(tile.entity.body);
  }
  s_tiles.tiles.clear();
}

void tile_manager_load() {
  // For better visualization 
  NKLevelFile* nklvl = &s_tiles.level_ref->nkbin;
  
  // Load the tiles
  
  s_tiles.tiles.resize(nklvl->tiles_count);
  for(nikola::sizei i = 0; i < s_tiles.tiles.size(); i++) {
    Tile* tile = &s_tiles.tiles[i];

    tile_create(tile,  
                s_tiles.level_ref, 
                (TileType)nklvl->tiles[i].tile_type, 
                nklvl->tiles[i].position);
  }
}

void tile_manager_save() {
  // For better visualization 
  NKLevelFile* nklvl = &s_tiles.level_ref->nkbin;
 
  // Save the tiles
  
  nklvl->tiles_count = s_tiles.tiles.size();
  for(nikola::sizei i = 0; i < s_tiles.tiles.size(); i++) {
    Tile* tile = &s_tiles.tiles[i];

    nklvl->tiles[i].position  = nikola::physics_body_get_position(tile->entity.body);
    nklvl->tiles[i].tile_type = (nikola::u8)tile->type;
  }
}

void tile_manager_update() {
  // @TODO: Please no. It works, but please no. It's SO bad-looking. 

  float step = TILE_SIZE;

  if(nikola::input_key_pressed(nikola::KEY_UP)) {
    s_tiles.debug_selection.x += step;
  }
  else if(nikola::input_key_pressed(nikola::KEY_DOWN)) {
    s_tiles.debug_selection.x -= step;
  }

  if(nikola::input_key_pressed(nikola::KEY_RIGHT)) {
    s_tiles.debug_selection.z += step;
  }
  else if(nikola::input_key_pressed(nikola::KEY_LEFT)) {
    s_tiles.debug_selection.z -= step;
  }

  if(nikola::input_key_down(nikola::KEY_LEFT_SHIFT) && nikola::input_key_pressed(nikola::KEY_1)) {
    s_tiles.tiles.resize(s_tiles.tiles.size() + 1);

    tile_create(&s_tiles.tiles[s_tiles.tiles.size() - 1], 
                s_tiles.level_ref, 
                TILE_ROAD, 
                s_tiles.debug_selection);
  }
  else if(nikola::input_key_down(nikola::KEY_LEFT_SHIFT) && nikola::input_key_pressed(nikola::KEY_2)) {
    s_tiles.tiles.resize(s_tiles.tiles.size() + 1);

    // We elevate the paviment a bit to make it look more "realistic"
    tile_create(&s_tiles.tiles[s_tiles.tiles.size() - 1], 
                s_tiles.level_ref, 
                TILE_PAVIMENT, 
                s_tiles.debug_selection + nikola::Vec3(0.0f, 0.3f, 0.0f));
  }
  else if(nikola::input_key_down(nikola::KEY_LEFT_SHIFT) && nikola::input_key_pressed(nikola::KEY_3)) {
    s_tiles.tiles.resize(s_tiles.tiles.size() + 1);

    tile_create(&s_tiles.tiles[s_tiles.tiles.size() - 1], 
                s_tiles.level_ref, 
                TILE_TUNNEL, 
                s_tiles.debug_selection);
  }
}

void tile_manager_render() {
  nikola::ResourceID mesh_id  = s_tiles.level_ref->resources[LEVEL_RESOURCE_CUBE];
  nikola::Transform transform = {}; 

  // Render tiles

  for(auto& tile : s_tiles.tiles) {
    transform = nikola::physics_body_get_transform(tile.entity.body);

    switch(tile.type) {
      case TILE_PAVIMENT:
        nikola::transform_scale(transform, nikola::collider_get_extents(tile.entity.collider));
        nikola::renderer_queue_mesh(mesh_id, transform, s_tiles.level_ref->resources[LEVEL_RESOURCE_MATERIAL_PAVIMENT]);
        break;
      case TILE_ROAD:
        nikola::transform_scale(transform, nikola::collider_get_extents(tile.entity.collider));
        nikola::renderer_queue_mesh(mesh_id, transform, s_tiles.level_ref->resources[LEVEL_RESOURCE_MATERIAL_ROAD]);
        break;
    }
  }
  
  // Render debug tile selection
  if(s_tiles.level_ref->debug_mode) {
    nikola::transform_translate(transform, s_tiles.debug_selection);
    nikola::transform_scale(transform, nikola::Vec3(TILE_SIZE, 1.0f, TILE_SIZE));
    nikola::renderer_debug_cube(transform, nikola::Vec4(1.0f, 0.0f, 1.0f, 0.2f));
  }
}

void tile_manager_render_gui() {
  if(ImGui::CollapsingHeader("Tiles")) {
    ImGui::Text("Tiles count: %zu", s_tiles.tiles.size());
   
    if(ImGui::Button("Clear tiles")) {
      s_tiles.tiles.clear();
    }

    for(nikola::sizei i = 0; i < s_tiles.tiles.size(); i++) {
      nikola::String name = ("Tile " + std::to_string(i)); 
      Entity* entity      = &s_tiles.tiles[i].entity;
      
      ImGui::SeparatorText(name.c_str());
      ImGui::PushID(name.c_str());

      // Position 
      nikola::Vec3 position = nikola::physics_body_get_position(entity->body);
      if(ImGui::DragFloat3("Position", &position[0], 0.1f)) {
        nikola::physics_body_set_position(entity->body, position);
        entity->start_pos = position;
      }
      
      // Scale
      nikola::Vec3 scale = nikola::collider_get_extents(entity->collider);
      if(ImGui::DragFloat3("Scale", &scale[0], 0.1f)) {
        nikola::collider_set_extents(entity->collider, scale);
      }
      
      // Rotation
      float rotation = nikola::physics_body_get_rotation(entity->body).w * nikola::RAD2DEG;
      if(ImGui::DragFloat("Rotation", &rotation, 45.0f)) {
        nikola::physics_body_set_rotation(entity->body, nikola::Vec3(0.0f, 1.0f, 0.0f), rotation * nikola::DEG2RAD);
      }

      // Active 
      ImGui::Checkbox("Active", &entity->is_active);

      // Type
      int type = (int)s_tiles.tiles[i].type;
      if(ImGui::Combo("Type", &type, "Road\0Paviment\0Railing\0\0")) {
        s_tiles.tiles[i].type = (TileType)type;
      }
      
      // Remove the end point
      if(ImGui::Button("Remove")) {
        s_tiles.tiles.erase(s_tiles.tiles.begin() + i);
      }
      
      ImGui::PopID();
    }
  }
}

/// Tile manager functions
/// ----------------------------------------------------------------------
