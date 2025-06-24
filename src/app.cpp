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
/// App functions 

nikola::App* app_init(const nikola::Args& args, nikola::Window* window) {
  // App init
  nikola::App* app = new nikola::App{};

  // Window init
  app->window = window;
  nikola::window_set_fullscreen(window, true);

  // GUI init
  nikola::gui_init(window);

  // Set some much needed physics settings
  nikola::physics_world_set_gravity(nikola::Vec3(0.0f));
  nikola::physics_world_set_iterations_count(5);

  // Levels init
  level_manager_init(window);
  
  // Dialogue manager init
  dialogue_manager_init("dialogue.txt");

  // Sates init
  Level* current_level = level_manager_get_current_level(); 
  state_manager_init(window, current_level->resources[LEVEL_RESOURCE_FONT]);

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
