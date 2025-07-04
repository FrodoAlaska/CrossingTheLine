#include "level.h"
#include "states/state.h"
#include "game_event.h"
#include "sound_manager.h"
#include "input_manager.h"
#include "resource_database.h"

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

static void on_key_collected(const GameEvent& event, void* dispatcher, void* listener) {
  Level* lvl = (Level*)listener;
  
  lvl->current_light_color = nikola::Vec3(0.0f);
}

static void on_pause_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  Level* lvl = (Level*)user_data;

  switch(layout.current_option) {
    case 0: // Back To Menu
      game_event_dispatch(GameEvent {
        .type       = GAME_EVENT_STATE_CHANGED, 
        .state_type = STATE_MENU 
      });

      lvl->current_lerp_point = lvl->lerp_points[LERP_POINT_DEFAULT];
      level_reset(lvl);
      break;
    case 1: // Quit
      nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
      break;
  }
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions

static void init_ui(Level* lvl) {
  // Text init
  UITextDesc text_desc = {
    .string = "PAUSED", 

    .font_id   = resource_database_get(RESOURCE_FONT),
    .font_size = 80.0f, 

    .anchor = UI_ANCHOR_TOP_CENTER, 
    .color  = nikola::Vec4(1.0f),
  };
  ui_text_create(&lvl->pause_text, lvl->window_ref, text_desc);

  // Layout init
  
  ui_layout_create(&lvl->pause_layout, 
                   lvl->window_ref,
                   resource_database_get(RESOURCE_FONT),
                   on_pause_layout_click_func,
                   lvl);

  ui_layout_begin(lvl->pause_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(lvl->pause_layout, "Back To Main Menu", 40.0f, nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
  ui_layout_push_text(lvl->pause_layout, "Quit", 40.0f, nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
  ui_layout_end(lvl->pause_layout);

  lvl->pause_layout.is_active = false;
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

  lvl->lerp_points[LERP_POINT_START]   = nikola::Vec3(-48.0f, 20.0f, 10.0f);
  lvl->lerp_points[LERP_POINT_WIN]     = nikola::Vec3(50.0f, 80.0f, 10.0f);
  lvl->lerp_points[LERP_POINT_LOSE]    = nikola::Vec3(-61.0f, -55.0f, 10.0f);
  lvl->lerp_points[LERP_POINT_DEFAULT] = nikola::Vec3(-48.0f, 180.0f, 10.0f);
  
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
  lvl->main_camera.yaw   = 0.6f;
  lvl->main_camera.pitch = -25.0f;
  lvl->main_camera.far   = 150.0f;

  // GUI camera init
  nikola::camera_create(&lvl->gui_camera, cam_desc);
  lvl->gui_camera.position = lvl->lerp_points[LERP_POINT_START]; 
  lvl->gui_camera.move_fn  = editor_camera_func; 
  lvl->gui_camera.far      = 500.0f;
  lvl->gui_camera.yaw      = 2.0f;
  lvl->gui_camera.pitch    = -10.5f;

  // Listen to events
  nikola::event_listen(nikola::EVENT_MOUSE_SCROLL_WHEEL, mouse_scroll_event, &lvl->gui_camera);
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_changed, lvl);
  game_event_listen(GAME_EVENT_COIN_COLLECTED, on_key_collected, lvl);

  // Current camera init
  lvl->frame.camera   = lvl->main_camera;
  lvl->current_camera = &lvl->main_camera;

  // Skybox init
  lvl->frame.skybox_id = resource_database_get(RESOURCE_SKYBOX);

  // UI init
  init_ui(lvl);

  // Entity manager init
  entity_manager_create(lvl);

  // Tiles init
  tile_manager_create(lvl);

  // Lights init
  lvl->frame.dir_light.direction = nikola::Vec3(-1.0f);
  lvl->frame.dir_light.color     = nikola::Vec3(0.5f);
  lvl->frame.ambient             = nikola::Vec3(0.4f);

  // Point light init
  lvl->current_light_color = nikola::Vec3(4.0f, 4.0f, 0.5f); 
  nikola::PointLight point = {
    .position = nikola::Vec3(2000.0f),
    .color    = lvl->current_light_color,
  };
  lvl->frame.point_lights.push_back(point);

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

  // Reset the light
  if(lvl->nkbin.has_coin) {
    lvl->frame.point_lights[0].position   = lvl->nkbin.coin_position;
    lvl->frame.point_lights[0].position.y = 2.0f;

    lvl->current_light_color = nikola::Vec3(4.0f, 4.0f, 0.5f);
  }
  else {
    lvl->frame.point_lights[0].position = nikola::Vec3(-2000.0f);
    lvl->current_light_color            = nikola::Vec3(0.0f);
  }

  // Load entities
  entity_manager_load();

  // Load tiles
  tile_manager_load();

  NIKOLA_PERF_TIMER_END(timer, (nikola::filepath_filename(path)).c_str());
  return true;
}

void level_destroy(Level* lvl) {
  NIKOLA_ASSERT(lvl, "Invalid level given to level_destroy");
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
  
  // Reset layout
  
  lvl->pause_layout.is_active = false;
  lvl->pause_text.color.a     = 0.0f;
  
  for(auto& txt : lvl->pause_layout.texts) {
    txt.color.a = 0.0f;
  }

  // Reset the entities
  entity_manager_reset();

  // Reset the physics world
  nikola::physics_world_set_paused(false);
}

void level_process_input(Level* lvl) {
  // Disable/enable the GUI
#if DISTRIBUTION_BUILD == 0
  if(nikola::input_key_pressed(nikola::KEY_F1)) {
    lvl->has_editor       = !lvl->has_editor;
    lvl->debug_mode       = lvl->has_editor;
    lvl->current_camera   = lvl->has_editor ? &lvl->gui_camera : &lvl->main_camera;

    nikola::physics_world_set_paused(lvl->has_editor);
    nikola::input_cursor_show(lvl->has_editor);
  }
#endif

  // Pause level
  if(input_manager_action_pressed(INPUT_ACTION_PAUSE)) {
    lvl->is_paused              = !lvl->is_paused; 
    lvl->pause_layout.is_active = lvl->is_paused;

    nikola::physics_world_set_paused(lvl->is_paused);
  }

  // Layout update
  ui_layout_update(lvl->pause_layout);

  // Update tiles
  if(lvl->has_editor) {
    tile_manager_process_input();
  }
}

void level_update(Level* lvl) {
  if(lvl->is_paused) {
    return;
  }

  float delta = nikola::niclock_get_delta_time() * 1.5f;

  // Keep lerping the camera which is honestly a bad idea
  nikola::Camera* camera = &lvl->main_camera;
  camera->position       = nikola::vec3_lerp(camera->position, lvl->current_lerp_point, delta);

  // Also not a good idea, but lerp the point light color as well
  nikola::PointLight* light = &lvl->frame.point_lights[0];
  light->color              = nikola::vec3_lerp(light->color, lvl->current_light_color, delta);

  // Update entities
  entity_manager_update();

  // Camera update
  lvl->frame.camera = *lvl->current_camera;
  nikola::camera_update(*lvl->current_camera);
}

void level_render_hud(Level* lvl) {
  if(!lvl->is_paused) {
    return;
  }

  ui_text_render_animation(lvl->pause_text, UI_TEXT_ANIMATION_BLINK, 8.0f);
  ui_layout_render_animation(lvl->pause_layout, UI_TEXT_ANIMATION_FADE_IN, 10.0f);
}

void level_render(Level* lvl) {
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
    ImGui::DragFloat3("Lerp point", &lvl->current_lerp_point[0], -0.1f);

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
      nikola::physics_world_set_paused(true);
    }
  }

  nikola::gui_end_panel();
}

/// Level functions
/// ----------------------------------------------------------------------
