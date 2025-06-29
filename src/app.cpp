#include "app.h"
#include "states/state.h"
#include "levels/level.h"
#include "resource_database.h"
#include "sound_manager.h"
#include "game_event.h"
#include "fog_shader.h"

#include <nikola/nikola.h>
#include <imgui/imgui.h>

/// ----------------------------------------------------------------------
/// Macros

#define INVOKE_STATE_CALLBACK(func, ...) if(func) func(##__VA_ARGS__) 

/// Macros
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// App
struct nikola::App {
  nikola::Window* window; 
  
  StateType current_state;
  StateDesc states[STATES_MAX];

  float fog_density      = 9.0f;
  nikola::Vec3 fog_color = nikola::Vec3(0.5f);
};
/// App
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void fog_pass_fn(const nikola::RenderPass* previous, nikola::RenderPass* current, void* user_data) {
  nikola::App* app = (nikola::App*)user_data;

  // Get both the previous pass's color buffer and depth-stencil buffers
  current->frame_desc.attachments[0] = previous->frame_desc.attachments[0];
  current->frame_desc.attachments[1] = previous->frame_desc.attachments[1];

  // Update shader uniforms
  
  nikola::Camera current_camera = level_manager_get_current_level()->frame.camera;
  nikola::ShaderContext* ctx    = nikola::resources_get_shader_context(current->shader_context_id);

  nikola::shader_context_set_uniform(ctx, "u_fog_density", app->fog_density);
  nikola::shader_context_set_uniform(ctx, "u_fog_color", app->fog_color);
}

static void on_state_change(const GameEvent& event, void* dispatcher, void* listener) {
  NIKOLA_ASSERT((event.state_type >= STATE_MENU && event.state_type < STATES_MAX), "Invalid State ID given to event");
  nikola::App* app = (nikola::App*)listener;

  // Reset the state before switching to it
  
  app->current_state = (StateType)event.state_type;
  INVOKE_STATE_CALLBACK(app->states[app->current_state].reset_func);

  GameEvent sound_event = {
    .type       = GAME_EVENT_SOUND_PLAYED, 
    .sound_type = SOUND_UI_TRANSITION
  };
  game_event_dispatch(sound_event);
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions

static void init_render_passes(nikola::App* app) {
  // Push the shader context to use
  nikola::ResourceID fog_shader_id     = nikola::resources_push_shader(nikola::RESOURCE_CACHE_ID, generate_fog_shader()); 
  nikola::ResourceID shader_context_id = nikola::resources_push_shader_context(nikola::RESOURCE_CACHE_ID, fog_shader_id);

  // Get the size of the window
  int width, height;
  nikola::window_get_size(app->window, &width, &height);

  // Fog pass init
  nikola::RenderPassDesc fog_pass = {
    .frame_size        = nikola::Vec2(width, height), 
    .clear_color       = nikola::Vec4(1.0f),
    .clear_flags       = (nikola::GFX_CLEAR_FLAGS_COLOR_BUFFER | nikola::GFX_CLEAR_FLAGS_DEPTH_BUFFER),
    .shader_context_id = shader_context_id,
  };
  fog_pass.targets.push_back(nikola::RenderTarget{
      .type   = nikola::GFX_TEXTURE_RENDER_TARGET, 
      .format = nikola::GFX_TEXTURE_FORMAT_RGBA8,
  });
  nikola::renderer_push_pass(fog_pass, fog_pass_fn, app);
}

static void init_states(nikola::App* app) {
  // Menu state init 
  StateDesc state_desc = {
    .init_func   = menu_state_init, 
    .reset_func  = menu_state_reset, 
    .input_func  = menu_state_process_input, 
    .render_func = menu_state_render,
  };
  app->states[STATE_MENU] = state_desc;

  // Level state init
  state_desc = {
    .init_func   = level_manager_init, 
    .reset_func  = nullptr, 
    .input_func  = level_manager_process_input, 
    .render_func = level_manager_render_hud,
  };
  app->states[STATE_LEVEL] = state_desc;

  // Won state init
  state_desc = {
    .init_func   = won_state_init, 
    .reset_func  = won_state_reset, 
    .input_func  = won_state_process_input, 
    .render_func = won_state_render,
  };
  app->states[STATE_WON] = state_desc;

  // Lost state init
  state_desc = {
    .init_func   = lost_state_init, 
    .reset_func  = lost_state_reset, 
    .input_func  = lost_state_process_input, 
    .render_func = lost_state_render,
  };
  app->states[STATE_LOST] = state_desc;

  // Credits state init
  state_desc = {
    .init_func   = credits_state_init, 
    .reset_func  = credits_state_reset, 
    .input_func  = credits_state_process_input, 
    .render_func = credits_state_render,
  };
  app->states[STATE_CREDITS] = state_desc;
  
  // States init
  for(nikola::sizei i = 0; i < STATES_MAX; i++) {
    INVOKE_STATE_CALLBACK(app->states[i].init_func, app->window, resource_database_get(RESOURCE_FONT));
  }

  // Current state init 
  app->current_state = STATE_MENU;
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
  nikola::window_set_fullscreen(window, true);

  // GUI init
  nikola::gui_init(window);

  // Set some much needed physics settings
  nikola::physics_world_set_gravity(nikola::Vec3(0.0f));
  nikola::physics_world_set_iterations_count(5);

  // @TODO: Render pass init
  // init_render_passes(app);

  // Resources init
  resource_database_init();

  // Sounds init
  sound_manager_init();

  // States init
  init_states(app);

  // Listen to events
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_change, app);

  return app;
}

void app_shutdown(nikola::App* app) {
  level_manager_shutdown();
  resource_database_shutdown();
  nikola::gui_shutdown();

  delete app;
}

void app_update(nikola::App* app, const nikola::f64 delta_time) {
  // Quit the application when the specified exit key is pressed
  if(nikola::input_key_pressed(nikola::KEY_ESCAPE)) {
    nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
    return;
  }

  // Update the level
  level_manager_update();

  // Update the current state
  INVOKE_STATE_CALLBACK(app->states[app->current_state].input_func);
}

void app_render(nikola::App* app) {
  nikola::renderer_begin(level_manager_get_current_level()->frame);
  level_manager_render();
  nikola::renderer_end();
  
  nikola::batch_renderer_begin();
  
  // Render HUDs
  INVOKE_STATE_CALLBACK(app->states[app->current_state].render_func);

  nikola::batch_renderer_end();
}

void app_render_gui(nikola::App* app) {
#if DISTRIBUTION_BUILD == 0
  nikola::gui_begin();
  
  // Level GUI
  level_manager_render_gui();
  
  if(level_manager_get_current_level()->has_editor) {
    nikola::gui_begin_panel("Post-Processing");
    ImGui::DragFloat3("Fog color", &app->fog_color[0], 0.1f);
    ImGui::DragFloat("Fog density", &app->fog_density, 0.1f);
    nikola::gui_end_panel();
  }

  nikola::gui_end();
#endif
}

/// App functions 
/// ----------------------------------------------------------------------
