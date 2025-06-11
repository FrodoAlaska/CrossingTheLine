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
  lvl->resources[LEVEL_RESOURCE_MATERIAL_ROAD]     = nikola::resources_push_material(lvl->resource_group, nikola::resources_get_id(lvl->resource_group, "road_1"));

  // Font init 
  lvl->resources[LEVEL_RESOURCE_FONT] = nikola::resources_get_id(nikola::RESOURCE_CACHE_ID, "iosevka_bold");

  // Models init

  lvl->resources[LEVEL_RESOURCE_CAR]   = nikola::resources_push_model(lvl->resource_group, "models/sedan.nbrmodel");
  lvl->resources[LEVEL_RESOURCE_TRUCK] = nikola::resources_push_model(lvl->resource_group, "models/delivery.nbrmodel");
  lvl->resources[LEVEL_RESOURCE_COIN]  = nikola::resources_push_model(lvl->resource_group, "models/gold_key.nbrmodel");
}

static void reset_level(Level* lvl) {
  // Reset variables
  lvl->is_paused = false; 
  lvl->has_won   = false; 
  lvl->has_lost  = false; 

  // Reset the player/camera
  lvl->main_camera.position  = lvl->nkbin.start_position;
  lvl->main_camera.is_active = true;

  // Reset the entities
  entity_manager_reset();
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

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
 
  // Entity manager init
  entity_manager_create(lvl);

  // Tiles init
  tile_manager_create(lvl);

  // Lights init
  lvl->frame.dir_light.direction = nikola::Vec3(-1.0f);
  lvl->frame.dir_light.color     = nikola::Vec3(1.0f);
  lvl->frame.ambient             = nikola::Vec3(0.5f);

  return lvl;
}

bool level_load(Level* lvl, const nikola::FilePath& path) {
  nikola::PerfTimer timer; 
  NIKOLA_PERF_TIMER_BEGIN(timer);

  // Variables init
  lvl->is_paused = false; 
  lvl->has_won   = false; 
  lvl->has_lost  = false; 

  // NKLevel init
  if(!nklvl_file_load(&lvl->nkbin, path)) {
    return false;
  }

  // Main camera init
  lvl->main_camera.position = lvl->nkbin.start_position;

  // Load entities
  entity_manager_load();

  // Load tiles
  tile_manager_load();

  NIKOLA_PERF_TIMER_END(timer, (nikola::filepath_filename(path) + " loaded").c_str());
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
 
  // Entities destroy
  entity_manager_destroy(); 

  // Tiles destroy
  tile_manager_destroy();
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

  // Update tiles
  if(lvl->has_editor) {
    tile_manager_update();
  }

  // Update entities
  entity_manager_update();

  // Camera update
  lvl->frame.camera = *lvl->current_camera;
  nikola::camera_update(*lvl->current_camera);
}

void level_render(Level* lvl) {
  nikola::ResourceID mesh_id  = lvl->resources[LEVEL_RESOURCE_CUBE];
  nikola::Transform transform = {};

  // Render entities
  entity_manager_render();

  // Render the tiles
  tile_manager_render(); 
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
  // @NOTE: Don't look at it too much. It's ugly, I know. 
  // It's only for dev builds. It won't actually be in the 
  // final build.

  nikola::String level_name = nikola::filepath_filename(lvl->nkbin.path);
  nikola::gui_begin_panel(level_name.c_str());

  // Camera  
  if(ImGui::CollapsingHeader("Camera")) {
    nikola::gui_edit_camera("Main camera", &lvl->main_camera);
  }

  // Entities 
  entity_manager_render_gui();

  // Tiles
  tile_manager_render_gui();

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
      nikola::filepath_set_filename(lvl->nkbin.path, lvl_path); 

      entity_manager_save();
      tile_manager_save();
      nklvl_file_save(lvl->nkbin);
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
