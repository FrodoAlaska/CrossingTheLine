#include "states/state.h"
#include "ui/ui.h"
#include "levels/level.h"
#include "game_event.h"
#include "sound_manager.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Consts

const nikola::sizei TEXTS_MAX = 10;

/// Consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// CreditsState
struct CreditsState {
  UIText texts[TEXTS_MAX]; 
  nikola::sizei total_texts = 1;

  UILayout layout; 
};

static CreditsState s_credits;
/// WonState
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_credits_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  switch(layout.current_option) {
    case 0: // Relive
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

static void on_state_change(const GameEvent& event, void* dispatcher, void* listener) {
  if(event.state_type != STATE_CREDITS) {
    return;
  }

  // Play some nice music
  GameEvent sound_event = {
    .type       = GAME_EVENT_MUSIC_PLAYED, 
    .sound_type = SOUND_HUB,
  };
  game_event_dispatch(sound_event);
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Credits state functions

void credits_state_init(nikola::Window* window, const nikola::ResourceID& font_id) {
  // Title init 
  UITextDesc text_desc = {
    .font_id   = font_id,
    .font_size = 50.0f,
    .anchor = UI_ANCHOR_TOP_CENTER, 
    .color  = nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f),
  };

  // Other texts init

  nikola::String strings[] = {
    "And thus you have regained your power after crossing the line...",

    "\n\n\n\nArtists:",
    "\n\n\n\nKenney",
    "\n\n\n\nAqibGulzar_41",
    "\n\n\n\nKonvenienZapps",
    "\n\n\n\ncgbookcase (Dorian Zgraggen)",
    "\n\n\n\nzfight",
    
    "\n\n\n\n\n\n\nMusicians:",
    "\n\n\n\n\n\n\nJoth",

    "\n\n\n\n\n\n\n\n\nMade by FrodoAlaska",
  };

  for(nikola::sizei i = 0; i < TEXTS_MAX; i++) {
    text_desc.string    = strings[i];
    text_desc.font_size = 40.0f; 
    text_desc.offset   += nikola::Vec2(0.0f, 40.0f); 

    ui_text_create(&s_credits.texts[i], window, text_desc);
  }

  // Layout init
  UILayout* layout = &s_credits.layout;
  ui_layout_create(layout, 
                   window, 
                   font_id,
                   on_credits_layout_click_func);
 
  ui_layout_begin(*layout, UI_ANCHOR_BOTTOM_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(*layout, "Relive", 40.0f, nikola::Vec4(0.0f, 1.0f, 0.0f, 0.0f));
  ui_layout_push_text(*layout, "Quit", 40.0f, nikola::Vec4(0.0f, 1.0f, 0.0f, 0.0f));
  ui_layout_end(*layout);

  // Listen to events
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_change);
}

void credits_state_reset() {
  // Reset the texts
  
  s_credits.total_texts = 1;
  for(nikola::sizei i = 0; i < TEXTS_MAX; i++) {
    s_credits.texts[i].color.a = 0.0f;
  }
  
  // Reset the layout
 
  s_credits.layout.is_active = false;
  for(auto& txt : s_credits.layout.texts) {
    txt.color.a = 0.0f;
  }
}

void credits_state_process_input() {
  s_credits.layout.is_active = (s_credits.total_texts >= TEXTS_MAX);

  // Layout update
  ui_layout_update(s_credits.layout);
}

void credits_state_render() {
  // Render the texts in order
  
  for(nikola::sizei i = 0; i < s_credits.total_texts; i++) {
    ui_text_render_animation(s_credits.texts[i], UI_TEXT_ANIMATION_FADE_IN, 12.0f);
  }

  if(s_credits.texts[s_credits.total_texts - 1].color.a >= 1.0f) {
    s_credits.total_texts++;
  }
  s_credits.total_texts = nikola::clamp_int(s_credits.total_texts, 0, TEXTS_MAX);
  
  // Render the layout
  ui_layout_render_animation(s_credits.layout, UI_TEXT_ANIMATION_FADE_IN, 11.0f);
}

/// Credits state functions
/// ----------------------------------------------------------------------
