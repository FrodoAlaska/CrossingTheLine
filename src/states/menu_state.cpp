#include "states/state.h"
#include "ui/ui.h"
#include "game_event.h"
#include "sound_manager.h"
#include "input_manager.h"
#include "levels/level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// MenuType
enum MenuType {
  MENU_MAIN = 0, 
  MENU_SETTINGS,

  MENUS_MAX = MENU_SETTINGS + 1,
};
/// MenuType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// MenuState
struct MenuState {
  int master_volume, music_volume, sfx_volume;
 
  UIText title; 

  MenuType current_layout = MENU_MAIN;
  UILayout layouts[MENUS_MAX]; 
};

static MenuState s_menu;
/// MenuState
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_menu_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  switch(layout.current_option) {
    case 0: // Start
      level_manager_reset();
      game_event_dispatch(GameEvent {
        .type       = GAME_EVENT_STATE_CHANGED, 
        .state_type = STATE_LEVEL 
      });
      break;
    case 1: // Settings
      s_menu.current_layout = MENU_SETTINGS;
      ui_text_set_string(s_menu.title, "Settings");
      break;
    case 2: // Quit
      nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
      break;
  }
}

static void on_settings_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  switch(layout.current_option) {
    case 3: { // Apply changes
      nkdata_file_set_volume_data((float)(s_menu.master_volume / 100.0f), 
                                  (float)(s_menu.music_volume / 100.0f), 
                                  (float)(s_menu.sfx_volume / 100.0f)); 
    } break;
    case 4: // To Main Menu
      s_menu.current_layout = MENU_MAIN;
      
      menu_state_reset();
      ui_text_set_string(s_menu.title, "Crossing The Line");
      break;
    default:
      break;
  }
}

static void on_state_changed(const GameEvent& event, void* dispatcher, void* listener) {
  if(event.state_type != STATE_MENU) {
    return;
  }

  game_event_dispatch(GameEvent {
    .type       = GAME_EVENT_MUSIC_PLAYED, 
    .sound_type = SOUND_HUB
  });
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Menu state functions

void menu_state_init(nikola::Window* window, const nikola::ResourceID& font_id) {
  // Volumes init
  float master, music, sfx;
  nkdata_file_get_volume_data(&master, &music, &sfx);

  // Convert the values from a range of [0.0f, 1.0f] to [0, 100]
  s_menu.master_volume = (int)(master * 100);
  s_menu.music_volume  = (int)(music * 100);
  s_menu.sfx_volume    = (int)(sfx * 100);
  
  // Title init 
  UITextDesc text_desc = {
    .string = "Crossing The Line",

    .font_id   = font_id,
    .font_size = 70.0f,

    .anchor = UI_ANCHOR_TOP_CENTER, 
    .color  = nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f),
  }; 
  ui_text_create(&s_menu.title, window, text_desc);
  
  // Main layout init 

  UILayout* menu_layout = &s_menu.layouts[MENU_MAIN];
  ui_layout_create(menu_layout, 
                   window, 
                   font_id,
                   on_menu_layout_click_func);

  ui_layout_begin(*menu_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(*menu_layout, "Start", 40.0f, nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
  ui_layout_push_text(*menu_layout, "Settings", 40.0f, nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
  ui_layout_push_text(*menu_layout, "Quit", 40.0f, nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
  ui_layout_end(*menu_layout);
  
  // Main layout init 

  UILayout* settings_layout = &s_menu.layouts[MENU_SETTINGS];
  ui_layout_create(settings_layout, 
                   window, 
                   font_id,
                   on_settings_layout_click_func);

  ui_layout_begin(*settings_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(*settings_layout, ("Master Volume: " + std::to_string(s_menu.master_volume)), 40.0f, nikola::Vec4(1.0f));
  ui_layout_push_text(*settings_layout, ("Music Volume: " + std::to_string(s_menu.music_volume)), 40.0f, nikola::Vec4(1.0f));
  ui_layout_push_text(*settings_layout, ("SFX Volume: " + std::to_string(s_menu.sfx_volume)), 40.0f, nikola::Vec4(1.0f));
  ui_layout_end(*settings_layout);
  
  ui_layout_begin(*settings_layout, UI_ANCHOR_BOTTOM_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(*settings_layout, "Apply Changes", 40.0f, nikola::Vec4(1.0f));
  ui_layout_push_text(*settings_layout, "Back To Menu", 40.0f, nikola::Vec4(1.0f));
  ui_layout_end(*settings_layout);

  // Play some cool music
  game_event_dispatch(GameEvent {
    .type       = GAME_EVENT_MUSIC_PLAYED, 
    .sound_type = SOUND_HUB
  });

  // Listen to events
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_changed);
}

void menu_state_reset() {
  s_menu.title.color.a = 0.0f;

  // Main layout reset
  for(auto& txt : s_menu.layouts[MENU_MAIN].texts) {
    txt.color.a = 0.0f;
  }
}

void menu_state_process_input() {
  // Update the current layout
  ui_layout_update(s_menu.layouts[s_menu.current_layout]);

  // The functionality below is only available on the settings menu
  if(s_menu.current_layout != MENU_SETTINGS) {
    return;
  }

  // These are the keys will control the settings "knobs"
  int step; 
  if(input_manager_action_pressed(INPUT_ACTION_NAVIGATE_LEFT)) {
    step = -10; 
    
    game_event_dispatch(GameEvent {
      .type       = GAME_EVENT_SOUND_PLAYED, 
      .sound_type = SOUND_UI_NAVIGATE, 
    });
  }
  else if(input_manager_action_pressed(INPUT_ACTION_NAVIGATE_RIGHT)) {
    step = 10; 
    
    game_event_dispatch(GameEvent {
      .type       = GAME_EVENT_SOUND_PLAYED, 
      .sound_type = SOUND_UI_NAVIGATE, 
    });
  }
  else {
    return;
  }

  switch(s_menu.layouts[MENU_SETTINGS].current_option) {
    case 0: // Master volume
      s_menu.master_volume += step;
      ui_text_set_string(s_menu.layouts[MENU_SETTINGS].texts[0], 
                         ("Master Volume: " + std::to_string(s_menu.master_volume)));
      break;
    case 1: // Music volume
      s_menu.music_volume += step; 
      ui_text_set_string(s_menu.layouts[MENU_SETTINGS].texts[1], 
                         ("Music Volume: " + std::to_string(s_menu.music_volume)));
      break;
    case 2: // SFX volume
      s_menu.sfx_volume += step; 
      ui_text_set_string(s_menu.layouts[MENU_SETTINGS].texts[2], 
                         ("SFX Volume: " + std::to_string(s_menu.sfx_volume)));
      break;
    default:
      break;
  }
}

void menu_state_render() {
  ui_text_render_animation(s_menu.title, UI_TEXT_ANIMATION_FADE_IN, 12.0f);
  ui_layout_render_animation(s_menu.layouts[s_menu.current_layout], UI_TEXT_ANIMATION_FADE_IN, 12.0f);
}

/// Menu state functions
/// ----------------------------------------------------------------------
