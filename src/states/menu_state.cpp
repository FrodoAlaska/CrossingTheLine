#include "states/state.h"
#include "ui/ui.h"
#include "game_event.h"
#include "sound_manager.h"
#include "levels/level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// MenuState
struct MenuState {
  UIText title; 
  UILayout layout; 
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
    case 1: // Quit
      nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
      break;
  }
}

static void on_state_changed(const GameEvent& event, void* dispatcher, void* listener) {
  if(event.state_type != STATE_MENU) {
    return;
  }

  game_event_dispatch(GameEvent {
    .type       = GAME_EVENT_MUSIC_PLAYED, 
    .sound_type = SOUND_MUSIC_WON
  });
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Menu state functions

void menu_state_init(nikola::Window* window, const nikola::ResourceID& font_id) {
  // Title init 
  UITextDesc text_desc = {
    .string = "Crossing The Line",

    .font_id   = font_id,
    .font_size = 70.0f,

    .anchor = UI_ANCHOR_TOP_CENTER, 
    .color  = nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f),
  }; 
  ui_text_create(&s_menu.title, window, text_desc);
  
  // Layout init 

  UILayout* menu_layout = &s_menu.layout;
  ui_layout_create(menu_layout, 
                   window, 
                   font_id,
                   on_menu_layout_click_func);

  ui_layout_begin(*menu_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(*menu_layout, "Start", 40.0f, nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
  ui_layout_push_text(*menu_layout, "Quit", 40.0f, nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
  ui_layout_end(*menu_layout);

  // Play some cool music
  game_event_dispatch(GameEvent {
    .type       = GAME_EVENT_MUSIC_PLAYED, 
    .sound_type = SOUND_MUSIC_WON
  });

  // Listen to events
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_changed);
}

void menu_state_reset() {
  s_menu.title.color.a = 0.0f;
  for(auto& txt : s_menu.layout.texts) {
    txt.color.a = 0.0f;
  }
}

void menu_state_process_input() {
  ui_layout_update(s_menu.layout);
}

void menu_state_render() {
  ui_text_render_animation(s_menu.title, UI_TEXT_ANIMATION_BLINK, 10.0f);
  ui_layout_render_animation(s_menu.layout, UI_TEXT_ANIMATION_FADE_IN, 10.0f);
}

/// Menu state functions
/// ----------------------------------------------------------------------
