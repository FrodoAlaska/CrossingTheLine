#include "level.h"
#include "game_event.h"
#include "fog_shader.h"
#include "sound_manager.h"
#include "state_manager.h"

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
/// LerpPointType
enum LerpPointType {
  LERP_POINT_START = 0, 
  LERP_POINT_WIN,
  LERP_POINT_LOSE,
  LERP_POINT_DEFAULT,
};
/// LerpPointType
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
    cam->position.y += -event.mouse_scroll_value;
  } 

  return true;
}

static void editor_camera_func(nikola::Camera& camera) {
  float speed = 50.0f * nikola::niclock_get_delta_time();

  if(nikola::input_key_down(nikola::KEY_LEFT_ALT)) {
    nikola::Vec2 mouse_offset; 
    nikola::input_mouse_offset(&mouse_offset.x, &mouse_offset.y);
  
    camera.yaw   = mouse_offset.x * camera.sensitivity;
    camera.pitch = mouse_offset.y * camera.sensitivity;
    camera.pitch = nikola::clamp_float(camera.pitch, -nikola::CAMERA_MAX_DEGREES, nikola::CAMERA_MAX_DEGREES);
    
    nikola::input_cursor_show(false);
  } 
  else if(nikola::input_key_released(nikola::KEY_LEFT_ALT)) {
    nikola::input_cursor_show(true);
  }

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

static void on_state_changed(const GameEvent& event, void* dispatcher, void* listener) {
  Level* lvl = (Level*)listener;
  
  switch(event.state_type) {
    case STATE_LEVEL:
    case STATE_HUB:
      lvl->current_lerp_point = lvl->lerp_points[LERP_POINT_START];
      break;
    case STATE_WON:
      lvl->current_lerp_point = lvl->lerp_points[LERP_POINT_WIN];
      break;
    case STATE_LOST:
      lvl->current_lerp_point = lvl->lerp_points[LERP_POINT_LOSE];
      break;
    default:
      break;
  }
}

/// Callbacks
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
  lvl->frame.skybox_id = nikola::resources_push_skybox(lvl->resource_group, "cubemaps/dreamy_sky.nbrcubemap");

  // Meshes init
  lvl->resources[LEVEL_RESOURCE_CUBE] = nikola::resources_push_mesh(lvl->resource_group, nikola::GEOMETRY_CUBE);

  // Materials init
  
  lvl->resources[LEVEL_RESOURCE_MATERIAL_PAVIMENT] = nikola::resources_push_material(lvl->resource_group, nikola::resources_get_id(lvl->resource_group, "paviment"));
  lvl->resources[LEVEL_RESOURCE_MATERIAL_ROAD]     = nikola::resources_push_material(lvl->resource_group, nikola::resources_get_id(lvl->resource_group, "road"));

  // Models init

  lvl->resources[LEVEL_RESOURCE_CAR]   = nikola::resources_push_model(lvl->resource_group, "models/sedan.nbrmodel");
  lvl->resources[LEVEL_RESOURCE_TRUCK] = nikola::resources_push_model(lvl->resource_group, "models/delivery.nbrmodel");
  lvl->resources[LEVEL_RESOURCE_COIN]  = nikola::resources_push_model(lvl->resource_group, "models/gold_key.nbrmodel");
 
  // Sounds init
  
  nikola::resources_push_dir(lvl->resource_group, "audio");
 
  lvl->resources[LEVEL_RESOURCE_SOUND_DEATH]       = nikola::resources_get_id(lvl->resource_group, "sfx_death");
  lvl->resources[LEVEL_RESOURCE_SOUND_KEY_COLLECT] = nikola::resources_get_id(lvl->resource_group, "sfx_key_collect");
  lvl->resources[LEVEL_RESOURCE_SOUND_WIN]         = nikola::resources_get_id(lvl->resource_group, "sfx_win");
  lvl->resources[LEVEL_RESOURCE_SOUND_FAIL_INPUT]  = nikola::resources_get_id(lvl->resource_group, "sfx_fail_input");
  
  lvl->resources[LEVEL_RESOURCE_SOUND_UI_CLICK]      = nikola::resources_get_id(lvl->resource_group, "sfx_ui_click");
  lvl->resources[LEVEL_RESOURCE_SOUND_UI_NAVIGATE]   = nikola::resources_get_id(lvl->resource_group, "sfx_ui_navigate");
  lvl->resources[LEVEL_RESOURCE_SOUND_UI_TRANSITION] = nikola::resources_get_id(lvl->resource_group, "sfx_transition");
  
  lvl->resources[LEVEL_RESOURCE_SOUND_TILE_ROAD]     = nikola::resources_get_id(lvl->resource_group, "sfx_road");
  lvl->resources[LEVEL_RESOURCE_SOUND_TILE_PAVIMENT] = nikola::resources_get_id(lvl->resource_group, "sfx_paviment");
  
  lvl->resources[LEVEL_RESOURCE_MUSIC_AMBIANCE] = nikola::resources_get_id(lvl->resource_group, "music_ambiance");
  lvl->resources[LEVEL_RESOURCE_MUSIC_HUB]      = nikola::resources_get_id(lvl->resource_group, "music_nocturne");

  // Font init 
  lvl->resources[LEVEL_RESOURCE_FONT] = nikola::resources_get_id(nikola::RESOURCE_CACHE_ID, "iosevka_bold");
}

static void lerp_camera(Level* lvl) {
  nikola::Camera* camera = &lvl->main_camera;
  camera->position       = nikola::vec3_lerp(camera->position, lvl->current_lerp_point, nikola::niclock_get_delta_time() * 1.5f);
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Level functions

Level* level_create(nikola::Window* window) {
  Level* lvl = new Level{};

  // Window init
  lvl->window_ref = window;

  // Lerp points init

  lvl->lerp_points[LERP_POINT_START]   = nikola::Vec3(-61.0f, 55.0f, 16.0f);
  lvl->lerp_points[LERP_POINT_WIN]     = nikola::Vec3(50.0f, 80.0f, 10.0f);
  lvl->lerp_points[LERP_POINT_LOSE]    = nikola::Vec3(-61.0f, -55.0f, 10.0f);
  lvl->lerp_points[LERP_POINT_DEFAULT] = nikola::Vec3(100.0f, 55.0f, 10.0f);
  
  lvl->current_lerp_point = lvl->lerp_points[LERP_POINT_DEFAULT];

  // Main camera init
  nikola::CameraDesc cam_desc = {
    .position     = lvl->current_lerp_point,
    .target       = nikola::Vec3(0.0f, lvl->current_lerp_point.y, -3.0f),
    .up_axis      = nikola::Vec3(0.0f, 1.0f, 0.0f),
    .aspect_ratio = nikola::window_get_aspect_ratio(lvl->window_ref),
    .move_func    = nullptr,
  };
  nikola::camera_create(&lvl->main_camera, cam_desc);
  lvl->main_camera.yaw       = 0.2f;
  lvl->main_camera.pitch     = -40.0f;
  lvl->main_camera.far       = 150.0f;

  // GUI camera init
  nikola::camera_create(&lvl->gui_camera, cam_desc);
  lvl->gui_camera.position = lvl->lerp_points[LERP_POINT_START]; 
  lvl->gui_camera.move_fn  = editor_camera_func; 
  lvl->gui_camera.far      = 500.0f;
  lvl->gui_camera.yaw      = 0.0f;
  lvl->gui_camera.pitch    = -48.5f;
 
  // Listen to events
  nikola::event_listen(nikola::EVENT_MOUSE_SCROLL_WHEEL, mouse_scroll_event, &lvl->gui_camera);
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_changed, lvl);

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
  lvl->frame.dir_light.color     = nikola::Vec3(0.7f);
  lvl->frame.ambient             = nikola::Vec3(0.5f);

  return lvl;
}

bool level_load(Level* lvl, const nikola::FilePath& path) {
  NIKOLA_ASSERT(lvl, "Invalid level given to level_load");
  
  nikola::PerfTimer timer; 
  NIKOLA_PERF_TIMER_BEGIN(timer);

  // NKLevel init
  if(!nklvl_file_load(&lvl->nkbin, path)) {
    return false;
  }

  // Reset the camera
  lvl->main_camera.position = lvl->lerp_points[LERP_POINT_DEFAULT];

  // Load entities
  entity_manager_load();

  // Load tiles
  tile_manager_load();

  NIKOLA_PERF_TIMER_END(timer, (nikola::filepath_filename(path)).c_str());
  return true;
}

void level_destroy(Level* lvl) {
  NIKOLA_ASSERT(lvl, "Invalid level given to level_destroy");

  nikola::resources_destroy_group(lvl->resource_group);
  delete lvl;
}

void level_unload(Level* lvl) {
  NIKOLA_ASSERT(lvl, "Invalid level given to level_unload");
 
  // Entities destroy
  entity_manager_destroy(); 

  // Tiles destroy
  tile_manager_destroy();
}

void level_reset(Level* lvl) {
  // Reset variables
  lvl->is_paused = false; 

  // Reset the entities
  entity_manager_reset();
}

void level_update(Level* lvl) {
  // Take input
  
  // Disable/enable the GUI
#if NIKOLA_BUILD_DEBUG == 1
  if(nikola::input_key_pressed(nikola::KEY_F1)) {
    lvl->has_editor       = !lvl->has_editor;
    lvl->debug_mode       = lvl->has_editor;
    lvl->current_camera   = lvl->has_editor ? &lvl->gui_camera : &lvl->main_camera;

    nikola::physics_world_set_paused(lvl->has_editor);
    nikola::input_cursor_show(lvl->has_editor);
  }
#endif
 
  // Update state

  // Keep lerping the camera which is honestly a bad idea
  lerp_camera(lvl);

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

void level_render_gui(Level* lvl) { 
  // @NOTE: Don't look at it too much. It's ugly, I know. 
  // It's only for dev builds. It won't actually be in the 
  // final build.

  nikola::String level_name = nikola::filepath_filename(lvl->nkbin.path);
  nikola::gui_begin_panel(level_name.c_str());

  // Camera  
  if(ImGui::CollapsingHeader("Camera")) {
    nikola::gui_edit_camera("Main camera", &lvl->main_camera);
    nikola::gui_edit_camera("GUI camera", &lvl->gui_camera);
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
   
    static bool is_paused = false; 

    ImGui::SameLine();
    if(ImGui::Checkbox("Paused", &is_paused)) {
      nikola::physics_world_set_paused(is_paused);
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
      level_reset(lvl);
    }
  }

  nikola::gui_end_panel();
}

/// Level functions
/// ----------------------------------------------------------------------
