#include "entity.h"
#include "levels/level.h"
#include "sound_manager.h"
#include "resource_database.h"

#include <nikola/nikola.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

/// ----------------------------------------------------------------------
/// TileManager
struct TileManager {
  Level* level_ref;

  TileType selected_type     = TILE_ROAD;
  nikola::Vec3 selected_size = nikola::Vec3(TILE_SIZE, 1.0f, TILE_SIZE); 

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

void tile_manager_process_input() {
  // @TODO: Please no. It works, but please no. It's SO bad-looking. 

  static float move_step = TILE_SIZE;

  if(nikola::input_key_pressed(nikola::KEY_UP)) {
    s_tiles.debug_selection.x += move_step;
  }
  else if(nikola::input_key_pressed(nikola::KEY_DOWN)) {
    s_tiles.debug_selection.x -= move_step;
  }

  if(nikola::input_key_pressed(nikola::KEY_RIGHT)) {
    s_tiles.debug_selection.z += move_step;
  }
  else if(nikola::input_key_pressed(nikola::KEY_LEFT)) {
    s_tiles.debug_selection.z -= move_step;
  }

  if(!nikola::input_key_down(nikola::KEY_LEFT_SHIFT)) {
    return;
  }

  nikola::Vec3 position = s_tiles.debug_selection;

  // Road
  if(nikola::input_key_pressed(nikola::KEY_1)) {
    s_tiles.selected_type = TILE_ROAD;
    s_tiles.selected_size = nikola::Vec3(TILE_SIZE, 1.0f, TILE_SIZE);
    
    move_step = 2.0f; 
  }
  // Paviment
  else if(nikola::input_key_pressed(nikola::KEY_2)) {
    s_tiles.selected_type = TILE_PAVIMENT;
    s_tiles.selected_size = nikola::Vec3(TILE_SIZE, 1.0f, TILE_SIZE);

    move_step   = 2.0f; 
    position.y += 0.3f; // We elevate the paviment a bit to make it look more "realistic"
  }
  // Cone
  else if(nikola::input_key_pressed(nikola::KEY_3)) {
    s_tiles.selected_type = TILE_CONE;
    s_tiles.selected_size = nikola::Vec3(2.0f);
    
    move_step   = 1.0f; 
    position.y += 1.5f;
  }
  // One-way tunnel
  else if(nikola::input_key_pressed(nikola::KEY_4)) {
    s_tiles.selected_type = TILE_TUNNEL_ONE_WAY;
    s_tiles.selected_size = nikola::Vec3(1.5f, 2.0f, 1.0f);

    move_step = 2.0f; 
    position += nikola::Vec3(0.0f, 8.7f, 0.0f);
  }
  // Two-way tunnel
  else if(nikola::input_key_pressed(nikola::KEY_5)) {
    s_tiles.selected_type = TILE_TUNNEL_TWO_WAY;
    s_tiles.selected_size = nikola::Vec3(2.0f, 2.0f, 1.0f);
    
    move_step = 2.0f; 
    position += nikola::Vec3(-4.0f, 11.5f, 0.0f);
  }
  // Three-way tunnel
  else if(nikola::input_key_pressed(nikola::KEY_6)) {
    s_tiles.selected_type = TILE_TUNNEL_THREE_WAY;
    s_tiles.selected_size = nikola::Vec3(3.0f, 2.0f, 1.0f);
    
    move_step = 2.0f; 
    position += nikola::Vec3(0.0f, 8.7f, 0.0f);
  }
   
  // Welcome to the family new tile

  if(nikola::input_key_pressed(nikola::KEY_ENTER)) {
    s_tiles.tiles.resize(s_tiles.tiles.size() + 1);
    tile_create(&s_tiles.tiles[s_tiles.tiles.size() - 1], s_tiles.level_ref, s_tiles.selected_type, s_tiles.debug_selection);
  }
}

void tile_manager_render() {
  nikola::ResourceID mesh_id  = resource_database_get(RESOURCE_CUBE);
  nikola::Transform transform = {}; 

  // Render tiles

  for(auto& tile : s_tiles.tiles) {
    transform = nikola::physics_body_get_transform(tile.entity.body);

    switch(tile.type) {
      case TILE_PAVIMENT:
        nikola::transform_scale(transform, nikola::collider_get_extents(tile.entity.collider));
        nikola::renderer_queue_mesh(mesh_id, transform, resource_database_get(RESOURCE_MATERIAL_PAVIMENT));
        break;
      case TILE_ROAD:
        nikola::transform_scale(transform, nikola::collider_get_extents(tile.entity.collider));
        nikola::renderer_queue_mesh(mesh_id, transform, resource_database_get(RESOURCE_MATERIAL_ROAD));
        break;
      case TILE_CONE:
        nikola::transform_scale(transform, nikola::Vec3(4.0f));
        nikola::renderer_queue_model(resource_database_get(RESOURCE_CONE), transform);
        break;
      case TILE_TUNNEL_ONE_WAY:
        nikola::transform_scale(transform, nikola::Vec3(1.5f, 2.0f, 1.0f));
        nikola::renderer_queue_model(resource_database_get(RESOURCE_TUNNEL), transform);
        break;
      case TILE_TUNNEL_TWO_WAY:
        nikola::transform_scale(transform, nikola::Vec3(2.0f, 2.0f, 1.0f));
        nikola::renderer_queue_model(resource_database_get(RESOURCE_TUNNEL), transform);
        break;
      case TILE_TUNNEL_THREE_WAY:
        nikola::transform_scale(transform, nikola::Vec3(3.0f, 2.0f, 1.0f));
        nikola::renderer_queue_model(resource_database_get(RESOURCE_TUNNEL), transform);
        break;
      default:
        break;
    }
  
    if(s_tiles.level_ref->debug_mode) {
      nikola::renderer_debug_collider(tile.entity.collider, nikola::Vec3(1.0f, 0.0f, 1.0f));
    }
  }
  
  // Render debug tile selection
  if(s_tiles.level_ref->debug_mode) {
    nikola::transform_translate(transform, s_tiles.debug_selection + nikola::Vec3(2.0f));
    nikola::transform_scale(transform, s_tiles.selected_size);

    nikola::renderer_debug_cube(transform, nikola::Vec4(1.0f, 0.0f, 1.0f, 0.2f));
  }
}

void tile_manager_render_gui() {
  if(ImGui::CollapsingHeader("Tiles")) {
    ImGui::Text("Tiles count: %zu", s_tiles.tiles.size());
   
    if(ImGui::Button("Clear tiles")) {
      s_tiles.tiles.clear();
    }
    
    // Filter
    static int filter_type = TILE_NONE; // A non-existent tile type
    ImGui::Combo("Filter by type",  
                  &filter_type, 
                  "Road\0Paviment\0Cone\0Tunnel (One way)\0Tunnel (Two way)\0Tunnel (Three way)\0All\0\0");

    for(nikola::sizei i = 0; i < s_tiles.tiles.size(); i++) {
      if(s_tiles.tiles[i].type != filter_type && filter_type != TILE_NONE) {
        continue;
      }
      
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
      
      // Collider extents
      nikola::gui_edit_collider("Collider", entity->collider); 

      // Rotation
      float rotation = nikola::physics_body_get_rotation(entity->body).w * nikola::RAD2DEG;
      if(ImGui::DragFloat("Rotation", &rotation, 45.0f)) {
        nikola::physics_body_set_rotation(entity->body, nikola::Vec3(0.0f, 1.0f, 0.0f), rotation * nikola::DEG2RAD);
      }

      // Active 
      ImGui::Checkbox("Active", &entity->is_active);

      // Type
      int type = (int)s_tiles.tiles[i].type;
      if(ImGui::Combo("Type", &type, "Road\0Paviment\0Cone\0Tunnel (One way)\0Tunnel (Two way)\0Tunnel (Three way)\0\0")) {
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
