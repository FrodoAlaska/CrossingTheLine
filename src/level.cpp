#include "level.h"

#include <nikola/nikola.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

/// ----------------------------------------------------------------------
/// Consts

const nikola::u8 NKLVL_VERSION_MAJOR = 0; 
const nikola::u8 NKLVL_VERSION_MINOR = 2; 

/// Consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions

static void init_resources(Level* lvl) {
  // Resource group init
  nikola::FilePath res_path = nikola::filepath_append(nikola::filesystem_current_path(), "res");
  lvl->resource_group       = nikola::resources_create_group("level_res", res_path);

  // Textures init
  nikola::resources_push_dir(lvl->resource_group, "textures");

  // Skybox init
  lvl->frame.skybox_id = nikola::resources_push_skybox(lvl->resource_group, "cubemaps/accurate_night.nbrcubemap");

  // Meshes init
  lvl->resources[LEVEL_RESOURCE_CUBE] = nikola::resources_push_mesh(lvl->resource_group, nikola::GEOMETRY_CUBE);

  // Materials init
  
  lvl->resources[LEVEL_RESOURCE_MATERIAL_PAVIMENT] = nikola::resources_push_material(lvl->resource_group, nikola::resources_get_id(lvl->resource_group, "paviment_2"));
  lvl->resources[LEVEL_RESOURCE_MATERIAL_ROAD]    = nikola::resources_push_material(lvl->resource_group, nikola::resources_get_id(lvl->resource_group, "road_1"));

  // Font init 
  lvl->resources[LEVEL_RESOURCE_FONT] = nikola::resources_get_id(nikola::RESOURCE_CACHE_ID, "iosevka_bold");

  // Models init

  lvl->resources[LEVEL_RESOURCE_CAR]   = nikola::resources_push_model(lvl->resource_group, "models/sedan.nbrmodel");
  lvl->resources[LEVEL_RESOURCE_TRUCK] = nikola::resources_push_model(lvl->resource_group, "models/truck.nbrmodel");
}

static void init_default_entities(Level* lvl) {
  // Player init 
  player_create(&lvl->player, lvl, lvl->main_camera.position);

  // Debug plane 
  
  entity_create(&lvl->debug_plane, 
                lvl, 
                nikola::Vec3(10.0f, -5.0f, 10.0f), 
                nikola::Vec3(74.0f, 1.0f, 74.0f), 
                ENTITY_OBJECT);
}

static void write_nklvl_file(Level* lvl) {
  // Open the file first
  nikola::File file; 
  if(!nikola::file_open(&file, lvl->path, (int)(nikola::FILE_OPEN_WRITE | nikola::FILE_OPEN_BINARY))) {
    NIKOLA_LOG_ERROR("Failed to save the level file at \'%s\'", lvl->path.c_str());
    return;
  }

  // Wrtie the versions
  nikola::file_write_bytes(file, &NKLVL_VERSION_MAJOR, sizeof(NKLVL_VERSION_MAJOR));
  nikola::file_write_bytes(file, &NKLVL_VERSION_MINOR, sizeof(NKLVL_VERSION_MINOR));

  // Wrtie the starting position 
  nikola::Vec3 start_pos = nikola::physics_body_get_position(lvl->player.body); 
  nikola::file_write_bytes(file, &start_pos[0], sizeof(nikola::Vec3));

  // Wrtie the end points
  
  nikola::sizei end_points_count = lvl->end_points.size(); 
  nikola::file_write_bytes(file, &end_points_count, sizeof(end_points_count));

  for(nikola::sizei i = 0; i < end_points_count; i++) {
    nikola::Vec3 pos   = nikola::physics_body_get_position(lvl->end_points[i].body);
    nikola::Vec3 scale = nikola::collider_get_extents(lvl->end_points[i].collider);

    nikola::file_write_bytes(file, &pos[0], sizeof(pos));
    nikola::file_write_bytes(file, &scale[0], sizeof(scale));
  }

  // Write the tiles 

  nikola::sizei tiles_count = lvl->tiles.size(); 
  nikola::file_write_bytes(file, &tiles_count, sizeof(tiles_count));

  for(auto& tile : lvl->tiles) {
    if(!tile.entity.is_active) {
      continue;
    }

    nikola::Vec3 pos     = nikola::physics_body_get_position(tile.entity.body);
    nikola::u8 tile_type = (nikola::u8)tile.type;

    nikola::file_write_bytes(file, &pos[0], sizeof(pos));
    nikola::file_write_bytes(file, &tile_type, sizeof(tile_type));
  }

  // Wrtie the vehicles

  nikola::sizei vehicles_count = lvl->vehicles.size(); 
  nikola::file_write_bytes(file, &vehicles_count, sizeof(vehicles_count));

  for(auto& v : lvl->vehicles) {
    nikola::Vec3 position   = nikola::physics_body_get_position(v.entity.body);
    nikola::u8 vehicle_type = (nikola::u8)v.type;

    nikola::file_write_bytes(file, &position[0], sizeof(position));
    nikola::file_write_bytes(file, &v.direction[0], sizeof(v.direction));
    nikola::file_write_bytes(file, &v.acceleration, sizeof(v.acceleration));
    nikola::file_write_bytes(file, &vehicle_type, sizeof(vehicle_type));
  }

  // Always remember to close the file
  nikola::file_close(file);
  NIKOLA_LOG_TRACE("Saved level file at \'%s\'", lvl->path.c_str());
}

static void reset_level(Level* lvl) {
  // Reset variables
  lvl->is_paused = false; 
  lvl->has_won   = false; 
  lvl->has_lost  = false; 

  // Reset the player/camera
  lvl->main_camera.position  = lvl->player.start_pos;
  lvl->main_camera.is_active = true;
  lvl->player.is_active      = true;

  // Reset the vehicles
  for(auto& v : lvl->vehicles) {
    nikola::physics_body_set_position(v.entity.body, v.entity.start_pos);
    vehicle_set_active(v, true);
  }
}

static void resolve_player_collisions(Entity* player, Entity* other) {
  Level* lvl = player->level_ref;

  // End point
  if(other->type == ENTITY_END_POINT) {
    lvl->has_won = true;
  }
  // Vehicle
  else if(other->type == ENTITY_VEHICLE) {
    lvl->has_lost = true;

    player->is_active          = false;
    lvl->main_camera.is_active = false;

    other->is_active = false; 
    nikola::physics_body_set_awake(other->body, false);
  }
}

static void resolve_vehicle_collisions(Entity* vehicle, Entity* other) {
  // End point
  if(other->type == ENTITY_END_POINT) {
    nikola::physics_body_set_position(vehicle->body, vehicle->start_pos);
  }
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_entity_collision(const nikola::CollisionPoint& point) {
  // Getting the entities
  Entity* entt_a = (Entity*)nikola::physics_body_get_user_data(point.body_a);
  Entity* entt_b = (Entity*)nikola::physics_body_get_user_data(point.body_b);
 
  // @TEMP: Yeah. Terrible. I know.

  // Player collisions
  
  if(entt_a->type == ENTITY_PLAYER) {
    resolve_player_collisions(entt_a, entt_b);
  }
  else if(entt_b->type == ENTITY_PLAYER) {
    resolve_player_collisions(entt_b, entt_a);
  }

  // Vehicle collisions
  
  if(entt_a->type == ENTITY_VEHICLE) {
    resolve_vehicle_collisions(entt_a, entt_b);
  }
  else if(entt_b->type == ENTITY_VEHICLE) {
    resolve_vehicle_collisions(entt_b, entt_a);
  }
}

static bool mouse_scroll_event(const nikola::Event& event, const void* dispatcher, const void* listener) {
  if(event.type != nikola::EVENT_MOUSE_SCROLL_WHEEL) {
    return false;
  }

  // Probably the camera. Hopefully. I think.
  nikola::Camera* cam = (nikola::Camera*)listener;

  // Apply the mouse scroll value only when the GUI is unfocused 
  // (to avoid both zooming and scrolling up and down the GUI window)
  if(!nikola::gui_is_focused()) {
    cam->zoom += -event.mouse_scroll_value;
  } 

  return true;
}

static void keyboard_camera_move_func(nikola::Camera& camera) {
  float speed = 20.0f * nikola::niclock_get_delta_time();

  nikola::Vec2 mouse_offset; 
  nikola::input_mouse_offset(&mouse_offset.x, &mouse_offset.y);

  camera.yaw   = mouse_offset.x * camera.sensitivity;
  camera.pitch = mouse_offset.y * camera.sensitivity;
  camera.pitch = nikola::clamp_float(camera.pitch, -nikola::CAMERA_MAX_DEGREES, nikola::CAMERA_MAX_DEGREES);

  // Move forward
  if(nikola::input_key_down(nikola::KEY_W)) {
    camera.position += nikola::Vec3(camera.front.x, 0.0f, camera.front.z) * speed;
  }
  // Move backwards
  else if(nikola::input_key_down(nikola::KEY_S)) {
    camera.position -= nikola::Vec3(camera.front.x, 0.0f, camera.front.z) * speed;
  }
 
  // Move right
  if(nikola::input_key_down(nikola::KEY_A)) {
    camera.position -= nikola::vec3_normalize(nikola::vec3_cross(camera.front, camera.up)) * speed;
  }
  // Move left
  else if(nikola::input_key_down(nikola::KEY_D)) {
    camera.position += nikola::vec3_normalize(nikola::vec3_cross(camera.front, camera.up)) * speed;
  }
}

static void stationary_camera_func(nikola::Camera& camera) {
  float speed = 50.0f * nikola::niclock_get_delta_time();
 
  // Clamp the zoom
  camera.zoom = nikola::clamp_float(camera.zoom, 1.0f, nikola::CAMERA_MAX_ZOOM);

  // Move forward
  if(nikola::input_key_down(nikola::KEY_W)) {
    camera.position += nikola::Vec3(camera.front.x, 0.0f, camera.front.z) * speed;
  }
  // Move backwards
  else if(nikola::input_key_down(nikola::KEY_S)) {
    camera.position -= nikola::Vec3(camera.front.x, 0.0f, camera.front.z) * speed;
  }
 
  // Move right
  if(nikola::input_key_down(nikola::KEY_A)) {
    camera.position -= nikola::vec3_normalize(nikola::vec3_cross(camera.front, camera.up)) * speed;
  }
  // Move left
  else if(nikola::input_key_down(nikola::KEY_D)) {
    camera.position += nikola::vec3_normalize(nikola::vec3_cross(camera.front, camera.up)) * speed;
  }
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Level functions

Level* level_create(nikola::Window* window) {
  Level* lvl = new Level{};

  // Window init
  lvl->window_ref = window;

  // Main camera init
  nikola::CameraDesc cam_desc = {
    .position     = nikola::Vec3(-22.0f, 0.0f, 9.0f),
    .target       = nikola::Vec3(0.0f, 0.0f, -3.0f),
    .up_axis      = nikola::Vec3(0.0f, 1.0f, 0.0f),
    .aspect_ratio = nikola::window_get_aspect_ratio(lvl->window_ref),
    .move_func    = keyboard_camera_move_func,
  };
  nikola::camera_create(&lvl->main_camera, cam_desc);

  // GUI camera init
  nikola::camera_create(&lvl->gui_camera, cam_desc);
  lvl->gui_camera.move_fn  = stationary_camera_func; 
  lvl->gui_camera.far      = 500.0f;
  lvl->gui_camera.yaw      = -0.20f;
  lvl->gui_camera.pitch    = -48.5f;
  lvl->gui_camera.position = nikola::Vec3(-77.3f, 80.0f, 8.3f);
 
  // Listen to events
  nikola::event_listen(nikola::EVENT_MOUSE_SCROLL_WHEEL, mouse_scroll_event, &lvl->gui_camera);

  // Current camera init
  lvl->frame.camera   = lvl->main_camera;
  lvl->current_camera = &lvl->main_camera;

  // Resource init
  init_resources(lvl);
 
  // Default entities init
  init_default_entities(lvl);

  // Lights init
  lvl->frame.dir_light.direction.y = -1.0f;

  // Physics world callback init
  nikola::physics_world_set_collision_callback(on_entity_collision, nullptr); 

  return lvl;
}

bool level_load(Level* lvl, const nikola::FilePath& path) {
  nikola::PerfTimer timer; 
  NIKOLA_PERF_TIMER_BEGIN(timer);

  // Path init (to save the file if needed later)
  lvl->path = path;
 
  // Variables init
  lvl->is_paused = false; 
  lvl->has_won   = false; 
  lvl->has_lost  = false; 
  
  // Open the file first
  nikola::File file; 
  if(!nikola::file_open(&file, lvl->path, (int)(nikola::FILE_OPEN_READ | nikola::FILE_OPEN_BINARY))) {
    NIKOLA_LOG_ERROR("Failed to read the level file at \'%s\'", lvl->path.c_str());
    return false;
  }

  // Read the versions
  nikola::u8 major_version, minor_version; 
  nikola::file_read_bytes(file, &major_version, sizeof(major_version));
  nikola::file_read_bytes(file, &minor_version, sizeof(minor_version));

  // Checking for the file's validity
  bool is_valid = (major_version == NKLVL_VERSION_MAJOR) && (NKLVL_VERSION_MINOR);
  NIKOLA_ASSERT(is_valid, "Found invalid level binary version in given path");

  // Read the starting position 
  nikola::Vec3 start_pos; 
  nikola::file_read_bytes(file, &start_pos[0], sizeof(start_pos));

  // Player init
  player_create(&lvl->player, lvl, start_pos);
  lvl->main_camera.position = start_pos;

  // Read the end points
  
  nikola::sizei end_points_count = 0; 
  nikola::file_read_bytes(file, &end_points_count, sizeof(end_points_count));
  lvl->end_points.resize(end_points_count);

  for(nikola::sizei i = 0; i < end_points_count; i++) {
    nikola::Vec3 pos, scale;

    nikola::file_read_bytes(file, &pos[0], sizeof(pos));
    nikola::file_read_bytes(file, &scale[0], sizeof(scale));

    entity_create(&lvl->end_points[i],
                  lvl,
                  pos,
                  scale, 
                  ENTITY_END_POINT);
  }

  // Read the tiles
  
  nikola::sizei tiles_count = 0; 
  nikola::file_read_bytes(file, &tiles_count, sizeof(tiles_count));
  lvl->tiles.resize(tiles_count);

  for(nikola::sizei i = 0; i < tiles_count; i++) {
    nikola::Vec3 pos;
    nikola::u8 tile_type; 

    nikola::file_read_bytes(file, &pos[0], sizeof(pos));
    nikola::file_read_bytes(file, &tile_type, sizeof(tile_type));

    tile_create(&lvl->tiles[i], 
                lvl, 
                (TileType)tile_type, 
                pos);
  }

  // Read the vehicles

  nikola::sizei vehicles_count = 0; 
  nikola::file_read_bytes(file, &vehicles_count, sizeof(vehicles_count));
  lvl->vehicles.resize(vehicles_count);

  for(nikola::sizei i = 0; i < vehicles_count; i++) {
    nikola::Vec3 pos, dir; 
    nikola::u8 vehicle_type;
    float accel;

    nikola::file_read_bytes(file, &pos[0], sizeof(pos));
    nikola::file_read_bytes(file, &dir[0], sizeof(dir));
    nikola::file_read_bytes(file, &accel, sizeof(accel));
    nikola::file_read_bytes(file, &vehicle_type, sizeof(vehicle_type));
    
    vehicle_create(&lvl->vehicles[i],
                   lvl,
                   (VehicleType)vehicle_type,
                   pos,
                   dir, 
                   accel);
  }

  // Always remember to close the file
  nikola::file_close(file);
  
  NIKOLA_PERF_TIMER_END(timer, (nikola::filepath_filename(lvl->path) + " loaded").c_str());
  return true;
}

void level_destroy(Level* lvl) {
  if(!lvl) {
    return;
  }

  nikola::resources_destroy_group(lvl->resource_group);
  delete lvl;
}

void level_unload(Level* lvl) {
  if(!lvl) {
    return;
  }

  // Player destroy
  nikola::physics_body_destroy(lvl->player.body);

  // End points destroy
  for(auto& point : lvl->end_points) {
    nikola::physics_body_destroy(point.body);
  }
  lvl->end_points.clear();
  
  // Tiles destroy
  for(auto& tile : lvl->tiles) {
    nikola::physics_body_destroy(tile.entity.body);
  }
  lvl->tiles.clear();

  // Vehicles destroy
  for(auto& v : lvl->vehicles) {
    nikola::physics_body_destroy(v.entity.body);
  }
  lvl->vehicles.clear();
}

void level_update(Level* lvl) {
  // Take input
  
  // Disable/enable the GUI
  if(nikola::input_key_pressed(nikola::KEY_F1)) {
    lvl->has_editor       = !lvl->has_editor;
    lvl->debug_mode       = lvl->has_editor;
    lvl->current_camera   = lvl->has_editor ? &lvl->gui_camera : &lvl->main_camera;
    
    nikola::physics_world_set_paused(lvl->has_editor);
    nikola::input_cursor_show(lvl->has_editor);
  }

  // Add a tile
  if(lvl->has_editor) {
    // @TODO: Please no. It works, but please no. It's SO bad-looking. 
    // Like, honestly, make a tile manager or something. This is AWFUL. 

    float step = TILE_SIZE;

    if(nikola::input_key_pressed(nikola::KEY_UP)) {
      lvl->debug_selection.x += step;
    }
    else if(nikola::input_key_pressed(nikola::KEY_DOWN)) {
      lvl->debug_selection.x -= step;
    }

    if(nikola::input_key_pressed(nikola::KEY_RIGHT)) {
      lvl->debug_selection.z += step;
    }
    else if(nikola::input_key_pressed(nikola::KEY_LEFT)) {
      lvl->debug_selection.z -= step;
    }

    lvl->debug_selection.x = nikola::clamp_float(lvl->debug_selection.x, -24.0f, 40.0f);
    lvl->debug_selection.z = nikola::clamp_float(lvl->debug_selection.z, -22.0f, 42.0f);

    if(nikola::input_key_down(nikola::KEY_LEFT_SHIFT) && nikola::input_key_pressed(nikola::KEY_Q)) {
      lvl->tiles.resize(lvl->tiles.size() + 1);
      tile_create(&lvl->tiles[lvl->tiles.size() - 1], lvl, TILE_ROAD, lvl->debug_selection);
    }
    else if(nikola::input_key_down(nikola::KEY_LEFT_SHIFT) && nikola::input_key_pressed(nikola::KEY_E)) {
      lvl->tiles.resize(lvl->tiles.size() + 1);

      // We elevate the paviment a bit to make it look more "realistic"
      tile_create(&lvl->tiles[lvl->tiles.size() - 1], lvl, TILE_PAVIMENT, lvl->debug_selection + nikola::Vec3(0.0f, 0.3f, 0.0f));
    }
  }

  // Toggle pause mode
  if(nikola::input_key_pressed(nikola::KEY_P)) {
    lvl->is_paused = !lvl->is_paused;
  }
 
  // Reset the level
  if(lvl->has_lost && nikola::input_key_pressed(nikola::KEY_R)) {
    reset_level(lvl);
  }

  // It's pretty obvious what this is, but I'm 
  // writing a comment here for better visualization.
  if(lvl->is_paused && lvl->has_lost) {
    return;
  }

  // Update state

  // Player update
  player_update(lvl->player);

  // Camera update
  lvl->frame.camera = *lvl->current_camera;
  nikola::camera_update(*lvl->current_camera);

  // Collision tests

  // Do a simple AABB test between the player and the end point of the level.
  //
  // @TEMP: Probably would be better to have a defining point with name for the end. 
  if(entity_aabb_test(lvl->player, lvl->end_points[0])) {
    lvl->has_won = true;
  }
}

void level_render(Level* lvl) {
  nikola::ResourceID mesh_id  = lvl->resources[LEVEL_RESOURCE_CUBE];
  nikola::Transform transform = {};

  // Render tiles

  for(auto& tile : lvl->tiles) {
    transform = nikola::physics_body_get_transform(tile.entity.body);
    nikola::transform_scale(transform, nikola::collider_get_extents(tile.entity.collider));

    // @TODO: There's probably a better way to do this...
    int res_index = (tile.type == TILE_PAVIMENT) ? LEVEL_RESOURCE_MATERIAL_PAVIMENT : LEVEL_RESOURCE_MATERIAL_ROAD;
    nikola::renderer_queue_mesh(mesh_id, transform, lvl->resources[res_index]);
  }

  // Render vehicles
  
  for(auto& v : lvl->vehicles) {
    transform = nikola::physics_body_get_transform(v.entity.body);
    nikola::transform_scale(transform, nikola::Vec3(4.0f));

    // @TODO: There's probably a better way to do this...
    int res_index = (v.type == VEHICLE_CAR) ? LEVEL_RESOURCE_CAR : LEVEL_RESOURCE_TRUCK;
    nikola::renderer_queue_model(lvl->resources[res_index], transform);

    if(lvl->debug_mode) {
      nikola::renderer_debug_collider(v.entity.collider, nikola::Vec3(1.0f, 0.0f, 0.0f));
    }
  }

  // Debug rendering
  
  if(lvl->debug_mode) {
    // Player
    transform = nikola::physics_body_get_transform(lvl->player.body);
    nikola::transform_scale(transform, nikola::Vec3(1.0f));
    nikola::renderer_queue_mesh(mesh_id, transform);

    // End points
    for(auto& point : lvl->end_points) {
      nikola::renderer_debug_collider(point.collider);
    }

    // Debug plane 
    nikola::renderer_debug_collider(lvl->debug_plane.collider);

    // Debug tile selection
    nikola::transform_translate(transform, lvl->debug_selection);
    nikola::transform_scale(transform, nikola::Vec3(TILE_SIZE, 1.0f, TILE_SIZE));
    nikola::renderer_debug_cube(transform, nikola::Vec4(1.0f, 0.0f, 1.0f, 0.2f));
  }
}

void level_render_hud(Level* lvl) {
  nikola::Font* font = nikola::resources_get_font(lvl->resources[LEVEL_RESOURCE_FONT]);

  int width, height;
  nikola::window_get_size(lvl->window_ref, &width, &height);

  if(lvl->has_lost) {
    nikola::Vec2 half_size   = nikola::Vec2(width, height) / 2.0f;
    nikola::Vec4 loser_coler = nikola::Vec4(1.0f, 0.0f, 0.0f, 1.0f);

    nikola::batch_render_text(font, "You Died!", half_size, 64.0f, loser_coler);
    nikola::batch_render_text(font, "[R] Replay", half_size + nikola::Vec2(0.0f, 86.0f), 64.0f, loser_coler);
  }
  else if(lvl->has_won) {
    nikola::Vec2 half_size    = nikola::Vec2(width, height) / 2.0f;
    nikola::Vec4 winner_coler = nikola::Vec4(0.0f, 1.0f, 0.0f, 1.0f);

    nikola::batch_render_text(font, "Congratulations!!!", half_size, 64.0f, winner_coler);
    nikola::batch_render_text(font, "[ENTER] Continue", half_size + nikola::Vec2(0.0f, 86.0f), 64.0f, winner_coler);
  }

  nikola::batch_render_fps(font, nikola::Vec2(10.0f, 30.0f), 32.0f, nikola::Vec4(1.0f));
}

void level_render_gui(Level* lvl) { 
  nikola::String level_name = nikola::filepath_filename(lvl->path);
  nikola::gui_begin_panel(level_name.c_str());

  // Camera  
  if(ImGui::CollapsingHeader("Camera")) {
    nikola::gui_edit_camera("Main camera", &lvl->main_camera);
  }

  // Player
  if(ImGui::CollapsingHeader("Player")) {
    nikola::gui_edit_physics_body("Player body", lvl->player.body);
    nikola::gui_edit_collider("Player collider", lvl->player.collider);
  }
  
  // Points
  if(ImGui::CollapsingHeader("Points")) {
    ImGui::Text("End points count: %zu", lvl->end_points.size());

    for(nikola::sizei i = 0; i < lvl->end_points.size(); i++) {
      nikola::String name = ("Point " + std::to_string(i)); 
      Entity* entity      = &lvl->end_points[i];
      
      ImGui::SeparatorText(name.c_str());
      ImGui::PushID(name.c_str());

      // Position 
      nikola::Vec3 position = nikola::physics_body_get_position(entity->body);
      if(ImGui::DragFloat3("Position", &position[0], 0.1f)) {
        nikola::physics_body_set_position(entity->body, position);
        entity->start_pos = position;
      }

      // Size
      nikola::Vec3 size = nikola::collider_get_extents(entity->collider);
      if(ImGui::DragFloat3("Extents", &size[0], 0.1f)) {
        nikola::collider_set_extents(entity->collider, size);
      }
      
      // Remove the end point
      if(ImGui::Button("Remove")) {
        lvl->end_points.erase(lvl->end_points.begin() + i);
      }
      
      ImGui::PopID();
    }
    
    ImGui::SeparatorText("Add options");

    // Position
    static nikola::Vec3 position = nikola::Vec3(10.0f, 0.0f, 10.0f);
    ImGui::DragFloat3("Position", &position[0], 0.1f);
    
    // Scale
    static nikola::Vec3 scale = nikola::Vec3(1.0f);
    ImGui::DragFloat3("Scale", &scale[0], 0.1f);

    // Add an end point
    if(ImGui::Button("Add end point")) {
      lvl->end_points.resize(lvl->end_points.size() + 1);
      entity_create(&lvl->end_points[lvl->end_points.size() - 1],
                    lvl,
                    position,
                    scale,
                    ENTITY_END_POINT);
    }
  }
  
  // Tiles
  if(ImGui::CollapsingHeader("Tiles")) {
    ImGui::Text("Tiles count: %zu", lvl->tiles.size());
    
    for(nikola::sizei i = 0; i < lvl->tiles.size(); i++) {
      nikola::String name = ("Tile " + std::to_string(i)); 
      Entity* entity      = &lvl->tiles[i].entity;
      
      ImGui::SeparatorText(name.c_str());
      ImGui::PushID(name.c_str());

      // Position 
      nikola::Vec3 position = nikola::physics_body_get_position(entity->body);
      if(ImGui::DragFloat3("Position", &position[0], TILE_SIZE)) {
        nikola::physics_body_set_position(entity->body, position);
        entity->start_pos = position;
      }

      // Type
      int type = (int)lvl->tiles[i].type;
      if(ImGui::Combo("Type", &type, "Road\0Paviment\0\0")) {
        lvl->tiles[i].type = (TileType)type;
      }
      
      // Remove the end point
      if(ImGui::Button("Remove")) {
        lvl->tiles.erase(lvl->tiles.begin() + i);
      }
      
      ImGui::PopID();
    }
  }

  // Vehicles
  if(ImGui::CollapsingHeader("Vehicles")) {
    ImGui::Text("Vehicles count: %zu", lvl->vehicles.size());
    for(nikola::sizei i = 0; i < lvl->vehicles.size(); i++) {
      nikola::String name  = ("Vehicle " + std::to_string(i)); 
      Entity* vehicle_entt = &lvl->vehicles[i].entity;
      
      ImGui::SeparatorText(name.c_str());
      ImGui::PushID(name.c_str());

      // Position 
      nikola::Vec3 position = nikola::physics_body_get_position(vehicle_entt->body);
      if(ImGui::DragFloat3("Position", &position[0], 0.1f)) {
        nikola::physics_body_set_position(vehicle_entt->body, position);
        vehicle_entt->start_pos = position;
      }

      // Size
      nikola::Vec3 size = nikola::collider_get_extents(vehicle_entt->collider);
      if(ImGui::DragFloat3("Extents", &size[0], 0.1f)) {
        nikola::collider_set_extents(vehicle_entt->collider, size);
      }
      
      // Collider offset
      nikola::Vec3 offset = nikola::collider_get_local_transform(vehicle_entt->collider).position;
      if(ImGui::DragFloat3("Collider offset", &offset[0], 0.1f)) {
        nikola::collider_set_local_position(vehicle_entt->collider, offset);
      }
      
      // Acceleration
      if(ImGui::DragFloat("Acceleration", &lvl->vehicles[i].acceleration, 0.1f)) {
        // @TEMP: Little hack because I'm lazy
        vehicle_set_active(lvl->vehicles[i], false); 
        vehicle_set_active(lvl->vehicles[i], true); 
      }

      // Direction 
      ImGui::DragFloat3("Direction", &lvl->vehicles[i].direction[0], 0.1f, -1.0f, 1.0f);

      // Active state
      if(ImGui::Checkbox("Active", &vehicle_entt->is_active)) {
        vehicle_set_active(lvl->vehicles[i], vehicle_entt->is_active);
      }

      // Remove the vehicle
      if(ImGui::Button("Remove")) {
        lvl->vehicles.erase(lvl->vehicles.begin() + i);
      }

      ImGui::PopID();
    }
      
    ImGui::SeparatorText("Add options");

    // Position
    static nikola::Vec3 position = nikola::Vec3(-11.0f, -1.5f, -32.0f);
    ImGui::DragFloat3("Position", &position[0], 0.1f);
    
    // Direction
    static nikola::Vec3 dir = nikola::Vec3(0.0f, 0.0f, 1.0f);
    ImGui::DragFloat3("Direction", &dir[0], 0.1f, -1.0f, 1.0f);

    // Type
    static int type = VEHICLE_CAR;
    ImGui::Combo("Type", &type, "Car\0Truck\0\0");

    // Add a vehicle
    if(ImGui::Button("Add vehicle")) {
      lvl->vehicles.resize(lvl->vehicles.size() + 1);

      vehicle_create(&lvl->vehicles[lvl->vehicles.size() - 1], 
                     lvl, 
                     (VehicleType)type, 
                     position, 
                     dir);
    }
  }
 
  // Lights
  if(ImGui::CollapsingHeader("Lights")) {
    // Ambiance
    ImGui::DragFloat3("Ambient", &lvl->frame.ambient[0], 0.1f);

    // Directional light
    nikola::gui_edit_directional_light("Directional", &lvl->frame.dir_light);

    // Point lights
    for(nikola::sizei i = 0; i < lvl->frame.point_lights.size(); i++) {
      nikola::String name = ("Point " + std::to_string(i));
      nikola::gui_edit_point_light(name.c_str(), &lvl->frame.point_lights[i]);
    }

    // Add a point light
    if(ImGui::Button("Add point light")) {
      lvl->frame.point_lights.push_back(nikola::PointLight {
        .position = lvl->frame.camera.position,
      });
    }
  }

  // Level options
  if(ImGui::CollapsingHeader("Level")) {
    // Name
    static nikola::String lvl_path = level_name;
    ImGui::InputText("Name", &lvl_path);

    // Debug mode
    ImGui::Checkbox("Debug Mode", &lvl->debug_mode);
   
    // Paused mode
    ImGui::SameLine();
    static bool paused = true; 
    if(ImGui::Checkbox("Paused", &paused)) {
      nikola::physics_world_set_paused(paused);
    }

    // Save the level
    if(ImGui::Button("Save level")) {
      nikola::filepath_set_filename(lvl->path, lvl_path); 
      write_nklvl_file(lvl);
    }

    // Reset the level
    ImGui::SameLine();
    if(ImGui::Button("Reset level")) {
      reset_level(lvl);
    }
  }

  nikola::gui_end_panel();
}

/// Level functions
/// ----------------------------------------------------------------------
