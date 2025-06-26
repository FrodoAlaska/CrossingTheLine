#include "states/state.h"
#include "ui/ui.h"
#include "levels/level.h"
#include "game_event.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// CreditsState
struct CreditsState {
  UIText title; 
  UILayout layout; 
};

static CreditsState s_credits;
/// WonState
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_credits_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  switch(layout.current_option) {
    case 0: // Replay
      game_event_dispatch(GameEvent {
        .type       = GAME_EVENT_STATE_CHANGED, 
        .state_type = STATE_LEVEL 
      });
      level_manager_reset();
      break;
    case 1: // Quit
      nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
      break;
  }
}


/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Credits state functions

void credits_state_init(nikola::Window* window, const nikola::ResourceID& font_id) {
  // Title init 
  UITextDesc text_desc = {
    .string = "Thanks for playing the game!",

    .font_id   = font_id,
    .font_size = 70.0f,

    .anchor = UI_ANCHOR_TOP_CENTER, 
    .color  = nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f),
  };
  ui_text_create(&s_credits.title, window, text_desc);

  // Layout init
  UILayout* layout = &s_credits.layout;
  ui_layout_create(layout, 
                   window, 
                   font_id,
                   on_credits_layout_click_func);
 
  ui_layout_begin(*layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(*layout, "Replay", 40.0f, nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
  ui_layout_push_text(*layout, "Quit", 40.0f, nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
  ui_layout_end(*layout);
}

void credits_state_reset() {
  s_credits.title.color.a = 0.0f;
  for(auto& txt : s_credits.layout.texts) {
    txt.color.a = 0.0f;
  }
}

void credits_state_process_input() {
  ui_layout_update(s_credits.layout);
}

void credits_state_render() {
  ui_text_render_animation(s_credits.title, UI_TEXT_ANIMATION_FADE_IN, 8.0f);
  ui_layout_render_animation(s_credits.layout, UI_TEXT_ANIMATION_BLINK, 15.0f);
}

/// Credits state functions
/// ----------------------------------------------------------------------
