#include "app.h"

#include <nikola/nikola.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

#include "levels/level.h"
#include "ui/ui.h"

/// ----------------------------------------------------------------------
/// Consts

const nikola::sizei LEVELS_MAX = 5;

/// Consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// App
struct nikola::App {
  nikola::Window* window; 

  nikola::FilePath level_paths[LEVELS_MAX];
  int current_level = 0; 
  Level* level      = nullptr;

  UIText menu_text;
};
/// App
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

  // UI text init
  UITextDesc text_desc = {
    .string    = "Crossing The Line",
    .font_id   = nikola::resources_get_id(nikola::RESOURCE_CACHE_ID, "iosevka_bold"),
    .font_size = 50.0f,
    .anchor    = UI_ANCHOR_TOP_CENTER,
    .color     = nikola::Vec4(1.0f, 0.0f, 0.0f, 0.0f),
  };
  ui_text_create(&app->menu_text, window, text_desc);

  return app;
}

void app_shutdown(nikola::App* app) {
  level_destroy(app->level);
  nikola::gui_shutdown();

  delete app;
}

static bool can_start = false;

void app_update(nikola::App* app, const nikola::f64 delta_time) {
  // Quit the application when the specified exit key is pressed
  if(nikola::input_key_pressed(nikola::KEY_ESCAPE)) {
    nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
    return;
  }


  // Update the current level
  app->level->main_camera.is_active = can_start; 
  level_update(app->level);

  // Level switching if the level is done

  if(!app->level->has_won) {
    return;
  }

  if(nikola::input_key_pressed(nikola::KEY_ENTER) && app->current_level < (LEVELS_MAX - 1)) {
    app->current_level++;

    level_unload(app->level);
    level_load(app->level, app->level_paths[app->current_level]);
  }
}

void app_render(nikola::App* app) {
  nikola::renderer_begin(app->level->frame);
  level_render(app->level);
  nikola::renderer_end();
  
  nikola::batch_renderer_begin();
  level_render_hud(app->level);
  ui_text_render_animation(app->menu_text, UI_TEXT_ANIMATION_FADE_IN, 30.0f);
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
