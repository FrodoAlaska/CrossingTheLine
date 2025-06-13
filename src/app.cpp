#include "app.h"
#include "levels/level.h"
#include "ui/ui.h"
#include "game_event.h"

#include <nikola/nikola.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
/// ----------------------------------------------------------------------
/// Consts

const nikola::sizei LEVELS_MAX = 5;

/// Consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// GameStateType
enum GameStateType {
  GAME_STATE_MENU, 
  GAME_STATE_LEVEL, 
  GAME_STATE_WON,
  GAME_STATE_LOST,
  
  GAME_STATES_MAX = GAME_STATE_LOST + 1,
};
/// GameStateType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// MenuOptionID
enum MenuOptionID {
  MENU_OPTION_START = 0, 
  MENU_OPTION_QUIT,
};
/// MenuOptionID
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// WonOptionID
enum WonOptionID {
  WON_OPTION_CONTINUE = 0, 
  WON_OPTION_MENU,
  WON_OPTION_QUIT,
};
/// WonOptionID
/// ----------------------------------------------------------------------

/// LostOptionID
enum LostOptionID {
  LOST_OPTION_RETRY = 0, 
  LOST_OPTION_MENU,
  LOST_OPTION_QUIT,
};
/// LostOptionID
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// GameState
struct GameState {
  UIText title;
  UILayout layout;
};
/// GameState
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// App
struct nikola::App {
  nikola::Window* window; 

  nikola::FilePath level_paths[LEVELS_MAX];
  int current_level = 0; 
  Level* level      = nullptr;

  GameStateType current_state = GAME_STATE_MENU;
  GameState game_states[GAME_STATES_MAX];
};
/// App
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_menu_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  nikola::App* app = (nikola::App*)user_data; 

  switch(layout.current_option) {
    case MENU_OPTION_START:
      level_reset(app->level);
      app->current_state = GAME_STATE_LEVEL;
      break;
    case MENU_OPTION_QUIT:
      nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
      break;
  }
}

static void on_won_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  nikola::App* app = (nikola::App*)user_data; 

  switch(layout.current_option) {
    case WON_OPTION_CONTINUE:
      level_unload(app->level);
      level_load(app->level, app->level_paths[++app->current_level]);
      app->current_state = GAME_STATE_LEVEL;
      break;
    case WON_OPTION_MENU:
      level_reset(app->level);
      app->current_state                = GAME_STATE_MENU; 
      app->level->main_camera.is_active = false;
      break;
    case WON_OPTION_QUIT:
      nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
      break;
  }
}

static void on_lost_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  nikola::App* app = (nikola::App*)user_data; 

  switch(layout.current_option) {
    case LOST_OPTION_RETRY:
      level_reset(app->level);
      app->current_state = GAME_STATE_LEVEL;
      break;
    case LOST_OPTION_MENU:
      level_reset(app->level);
      app->current_state                = GAME_STATE_MENU; 
      app->level->main_camera.is_active = false;
      break;
    case LOST_OPTION_QUIT:
      nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
      break;
  }
}

static bool on_state_change(const GameEventType type, void* dispatcher, void* listener) {
  nikola::App* app = (nikola::App*)listener;

  switch(type) {
    case GAME_EVENT_LEVEL_WON:
      app->current_state = GAME_STATE_WON;
      return true;
    case GAME_EVENT_LEVEL_LOST:
      app->current_state = GAME_STATE_LOST;
      return true;
    default:
      return false;
  }
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions

static void init_resources(nikola::App* app) {
  // Font init
  nikola::resources_push_font(nikola::RESOURCE_CACHE_ID, "fonts/iosevka_bold.nbrfont");
}

static void init_levels(nikola::App* app) {
  nikola::FilePath current_path = nikola::filesystem_current_path();

  // Level names init
  
  app->level_paths[0] = nikola::filepath_append(current_path, "levels/level_1.nklvl");
  app->level_paths[1] = nikola::filepath_append(current_path, "levels/level_2.nklvl");
  app->level_paths[2] = nikola::filepath_append(current_path, "levels/level_3.nklvl");;
  app->level_paths[3] = nikola::filepath_append(current_path, "levels/level_4.nklvl");;
  app->level_paths[4] = nikola::filepath_append(current_path, "levels/level_5.nklvl");;

  // Current level init
  
  app->level = level_create(app->window);
  level_load(app->level, app->level_paths[0]);
}

static void init_game_states(nikola::App* app) {
  // Menu state

  UILayout* menu_layout = &app->game_states[GAME_STATE_MENU].layout;
  ui_layout_create(menu_layout, 
                   app->window, 
                   nikola::resources_get_id(nikola::RESOURCE_CACHE_ID, "iosevka_bold"), 
                   on_menu_layout_click_func, 
                   app);
 
  UITextDesc text_desc = {
    .string = "Crossing The Line",

    .font_id   = nikola::resources_get_id(nikola::RESOURCE_CACHE_ID, "iosevka_bold"),
    .font_size = 50.0f,

    .anchor = UI_ANCHOR_TOP_CENTER, 
    .color  = nikola::Vec4(1.0f, 0.0f, 0.0f, 1.0f),
  }; 
  ui_text_create(&app->game_states[GAME_STATE_MENU].title, app->window, text_desc);

  ui_layout_begin(*menu_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 30.0f));
  ui_layout_push_text(*menu_layout, "Start", 30.0f, nikola::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
  ui_layout_push_text(*menu_layout, "Quit", 30.0f, nikola::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
  ui_layout_end(*menu_layout);
 
  // Won state
  
  UILayout* won_layout = &app->game_states[GAME_STATE_WON].layout;
  ui_layout_create(won_layout, 
                   app->window, 
                   nikola::resources_get_id(nikola::RESOURCE_CACHE_ID, "iosevka_bold"), 
                   on_won_layout_click_func, 
                   app);
 
  ui_layout_begin(*won_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(*won_layout, "Continue", 40.0f, nikola::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
  ui_layout_end(*won_layout);
  
  // Lost state
  
  UILayout* lost_layout = &app->game_states[GAME_STATE_LOST].layout;
  ui_layout_create(lost_layout, 
                   app->window, 
                   nikola::resources_get_id(nikola::RESOURCE_CACHE_ID, "iosevka_bold"), 
                   on_lost_layout_click_func, 
                   app);
 
  ui_layout_begin(*lost_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(*lost_layout, "Retry", 40.0f, nikola::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
  ui_layout_end(*lost_layout);
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// App functions 

nikola::App* app_init(const nikola::Args& args, nikola::Window* window) {
  // App init
  nikola::App* app = new nikola::App{};

  // Window init
  app->window = window;
  nikola::window_set_position(window, 100, 100);

  // GUI init
  nikola::gui_init(window);

  // Resources init
  init_resources(app);

  // Levels init
  init_levels(app);

  // @TODO: This might be useless, but it's just for testing purposes
  nikola::physics_world_set_gravity(nikola::Vec3(0.0f));
  nikola::physics_world_set_iterations_count(5);

  // Listen to events
  game_event_listen(GAME_EVENT_LEVEL_WON, on_state_change, app);
  game_event_listen(GAME_EVENT_LEVEL_LOST, on_state_change, app);

  // Game sates init
  init_game_states(app);

  return app;
}

void app_shutdown(nikola::App* app) {
  level_destroy(app->level);
  nikola::gui_shutdown();

  delete app;
}

void app_update(nikola::App* app, const nikola::f64 delta_time) {
  // Quit the application when the specified exit key is pressed
  if(nikola::input_key_pressed(nikola::KEY_ESCAPE)) {
    nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
    return;
  }

  // Update the current state
  ui_layout_update(app->game_states[app->current_state].layout);

  // Update the current level
  level_update(app->level);
}

void app_render(nikola::App* app) {
  nikola::renderer_begin(app->level->frame);
  level_render(app->level);
  nikola::renderer_end();
  
  nikola::batch_renderer_begin();
  
  // Render the current state's layout
  
  ui_text_render(app->game_states[app->current_state].title);
  ui_layout_render(app->game_states[app->current_state].layout);

  nikola::batch_renderer_end();
}

void app_render_gui(nikola::App* app) {
  if(!app->level->has_editor) {
    return;
  }
 
  nikola::gui_begin();
 
  // Debug GUI (not gonna stay here for long)
  nikola::gui_debug_info();
  
  // Level GUI
  level_render_gui(app->level);  

  // Level select
  nikola::gui_begin_panel("Level select"); 
 
  if(ImGui::Combo("Select level", &app->current_level, "Level 1\0Level 2\0Level 3\0Level 4\0Level 5\0\0")) {
    level_unload(app->level);
    level_load(app->level, app->level_paths[app->current_level]);
  }

  nikola::gui_end_panel(); 

  nikola::gui_end();
}

/// App functions 
/// ----------------------------------------------------------------------
