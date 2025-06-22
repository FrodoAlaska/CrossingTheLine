#include "app.h"

#include <nikola/nikola_app.h>

// Yeah, unfortunate...
#if NIKOLA_PLATFORM_WINDOWS == 1 
#include <windows.h>
#endif

int main(int argc, char** argv) {
  // Some useful flags
  int win_flags = nikola::WINDOW_FLAGS_FOCUS_ON_CREATE | 
                  nikola::WINDOW_FLAGS_CENTER_MOUSE    |
                  nikola::WINDOW_FLAGS_HIDE_CURSOR;
  
  // App init
  nikola::AppDesc app_desc {
    .init_fn     = app_init,
    .shutdown_fn = app_shutdown,
    .update_fn   = app_update, 
    
    .render_fn     = app_render, 
    .render_gui_fn = app_render_gui, 

    .window_title  = "Game 1", 
    .window_width  = 1600, 
    .window_height = 900, 
    .window_flags  = win_flags,

    .args_values = argv, 
    .args_count  = argc,
  };

  // Engine init
  nikola::engine_init(app_desc);
 
  // Main engine loop
  nikola::engine_run();

  // Goodbye. We're done now.
  nikola::engine_shutdown();

  return 0;
}

// NIKOLA_MAIN(engine_main);
