#include "states/state.h"
#include "ui/ui.h"
#include "game_event.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// LostState
struct LostState {
  UIText title; 
  UILayout layout; 
};

static LostState s_lost;
/// WonState
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_lost_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  switch(layout.current_option) {
    case 0: // Retry
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

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Lost state functions

void lost_state_init(nikola::Window* window, const nikola::ResourceID& font_id) {
  // Title init 
  UITextDesc text_desc = {
    .string = "To give up is a virtue. Do not continue.\nGive it all up and live a peacful life.",

    .font_id   = font_id,
    .font_size = 50.0f,

    .anchor = UI_ANCHOR_TOP_CENTER, 
    .color  = nikola::Vec4(0.0f, 1.0f, 0.0f, 0.0f),
  };
  ui_text_create(&s_lost.title, window, text_desc);

  // Layout init
  UILayout* lost_layout = &s_lost.layout;
  ui_layout_create(lost_layout, 
                   window, 
                   font_id,
                   on_lost_layout_click_func);
 
  ui_layout_begin(*lost_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(*lost_layout, "Relive", 40.0f, nikola::Vec4(1.0f, 0.0f, 0.0f, 0.0f));
  ui_layout_push_text(*lost_layout, "Give up", 40.0f, nikola::Vec4(0.0f, 1.0f, 0.0f, 0.0f));
  ui_layout_end(*lost_layout);
}

void lost_state_reset() {
  s_lost.title.color.a = 0.0f;
  for(auto& txt : s_lost.layout.texts) {
    txt.color.a = 0.0f;
  }
}

void lost_state_process_input() {
  ui_layout_update(s_lost.layout);
}

void lost_state_render() {
  ui_text_render_animation(s_lost.title, UI_TEXT_ANIMATION_FADE_IN, 8.0f);
  ui_layout_render_animation(s_lost.layout, UI_TEXT_ANIMATION_FADE_IN, 12.0f);
}

/// Lost state functions
/// ----------------------------------------------------------------------
