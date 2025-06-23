#include "app.h"
#include "levels/level.h"
#include "ui/ui.h"
#include "game_event.h"
#include "fog_shader.h"
#include "sound_manager.h"
#include "state_manager.h"
#include "dialogue_manager.h"

#include <nikola/nikola.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

/// ----------------------------------------------------------------------
/// App
struct nikola::App {
  nikola::Window* window; 
};
/// App
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions

static void init_resources(nikola::App* app) {
  // Font init
  nikola::resources_push_font(nikola::RESOURCE_CACHE_ID, "fonts/iosevka_bold.nbrfont");

  // Shader contexts init
  nikola::ResourceID fog_shader_id = nikola::resources_push_shader(nikola::RESOURCE_CACHE_ID, get_fog_shader());
  nikola::resources_push_shader_context(nikola::RESOURCE_CACHE_ID, fog_shader_id);
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
  //nikola::window_set_fullscreen(window, true);
  nikola::window_set_position(window, 100, 100);

  // GUI init
  nikola::gui_init(window);

  // Resources init
  init_resources(app);

  // @TODO: This might be useless, but it's just for testing purposes
  nikola::physics_world_set_gravity(nikola::Vec3(0.0f));

  // Levels init
  level_manager_init(window);
  
  // Dialogue manager init
  dialogue_manager_init("res/dialogue.txt");

  // Sates init
  state_manager_init(window);

  return app;
}

void app_shutdown(nikola::App* app) {
  level_manager_shutdown();
  nikola::gui_shutdown();

  delete app;
}

void app_update(nikola::App* app, const nikola::f64 delta_time) {
  // Quit the application when the specified exit key is pressed
  if(nikola::input_key_pressed(nikola::KEY_ESCAPE)) {
    nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
    return;
  }

  // Update the layout states
  state_manager_update();

  // Update levels
  level_manager_update();
}

void app_render(nikola::App* app) {
  nikola::renderer_begin(level_manager_get_current_level()->frame);
  level_manager_render();
  nikola::renderer_end();
  
  nikola::batch_renderer_begin();
  
  // Render HUDs

  state_manager_render_hud();
  level_manager_render_hud();

  nikola::batch_renderer_end();
}

void app_render_gui(nikola::App* app) {
  nikola::gui_begin();
  
  // Level GUI
  level_manager_render_gui();  

  nikola::gui_end();
}

/// App functions 
/// ----------------------------------------------------------------------
