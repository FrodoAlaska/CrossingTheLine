#include "state_manager.h"
#include "levels/level.h"
#include "game_event.h"
#include "ui/ui.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// MenuOptionID
enum MenuOptionID {
  MENU_OPTION_START = 0, 
  MENU_OPTION_QUIT,
};
/// MenuOptionID
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// WonOptionID
enum WonOptionID {
  WON_OPTION_CONTINUE = 0, 
};
/// WonOptionID
/// ----------------------------------------------------------------------

/// LostOptionID
enum LostOptionID {
  LOST_OPTION_RETRY = 0, 
  LOST_OPTION_QUIT,
};
/// LostOptionID
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// StateEntry
struct StateEntry {
  UIText title;
  UILayout layout;
};
/// StateEntry
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// StateManager
struct StateManager {
  StateEntry* current_state = nullptr;
  StateEntry entries[STATES_MAX];
};

static StateManager s_manager;
/// StateManager
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_menu_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  switch(layout.current_option) {
    case MENU_OPTION_START:
      game_event_dispatch(GameEvent {
        .type       = GAME_EVENT_STATE_CHANGED, 
        .state_type = STATE_HUB 
      });
      break;
    case MENU_OPTION_QUIT:
      nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
      break;
  }
}

static void on_won_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  switch(layout.current_option) {
    case WON_OPTION_CONTINUE:
      level_manager_advance();
      break;
  }
}

static void on_lost_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  switch(layout.current_option) {
    case LOST_OPTION_RETRY:
      game_event_dispatch(GameEvent {
        .type       = GAME_EVENT_STATE_CHANGED, 
        .state_type = STATE_LEVEL 
      });
      break;
    case LOST_OPTION_QUIT:
      nikola::event_dispatch(nikola::Event{.type = nikola::EVENT_APP_QUIT});
      break;
  }
}

static void on_state_change(const GameEvent& event, void* dispatcher, void* listener) {
  NIKOLA_ASSERT((event.state_type >= STATE_MENU && event.state_type < STATES_MAX), "Invalid State ID given to event");

  // Reset the layout before switching to it
  
  s_manager.current_state = &s_manager.entries[event.state_type];

  s_manager.current_state->title.color.a = 0.0f;
  for(auto& txt : s_manager.current_state->layout.texts) {
    txt.color.a = 0.0f;
  }
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions

static void init_menu_layout(nikola::Window* window, const nikola::ResourceID& font_id) {
  UILayout* menu_layout = &s_manager.entries[STATE_MENU].layout;
  ui_layout_create(menu_layout, 
                   window, 
                   font_id,
                   on_menu_layout_click_func);
 
  UITextDesc text_desc = {
    .string = "Crossing The Line",

    .font_id   = font_id,
    .font_size = 80.0f,

    .anchor = UI_ANCHOR_TOP_CENTER, 
    .color  = nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f),
  }; 
  ui_text_create(&s_manager.entries[STATE_MENU].title, window, text_desc);

  ui_layout_begin(*menu_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(*menu_layout, "Start", 40.0f, nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
  ui_layout_push_text(*menu_layout, "Quit", 40.0f, nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
  ui_layout_end(*menu_layout);
}

static void init_won_layout(nikola::Window* window, const nikola::ResourceID& font_id) {
  UITextDesc text_desc = {
    .string = "You will regret it. Time and time again.\nWhatever you do. Whatever you achieve.\nYou will regret it.\n\nDo not exhaust yourself any further.",

    .font_id   = font_id,
    .font_size = 50.0f,

    .anchor = UI_ANCHOR_TOP_CENTER, 
    .color  = nikola::Vec4(1.0f, 0.0f, 0.0f, 0.0f),
  };
  ui_text_create(&s_manager.entries[STATE_WON].title, window, text_desc);

  UILayout* won_layout = &s_manager.entries[STATE_WON].layout;
  ui_layout_create(won_layout, 
                   window, 
                   font_id,
                   on_won_layout_click_func);
 
  ui_layout_begin(*won_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(*won_layout, "Continue the suffering", 35.0f, nikola::Vec4(1.0f, 0.0f, 0.0f, 0.0f));
  ui_layout_end(*won_layout);
}

static void init_lost_layout(nikola::Window* window, const nikola::ResourceID& font_id) {
  UITextDesc text_desc = {
    .string = "To give up is a virtue. Do not continue.\nGive it all up and live a life of peace.",

    .font_id   = font_id,
    .font_size = 50.0f,

    .anchor = UI_ANCHOR_TOP_CENTER, 
    .color  = nikola::Vec4(0.0f, 1.0f, 0.0f, 0.0f),
  };
  ui_text_create(&s_manager.entries[STATE_LOST].title, window, text_desc);
  
  UILayout* lost_layout = &s_manager.entries[STATE_LOST].layout;
  ui_layout_create(lost_layout, 
                   window, 
                   font_id,
                   on_lost_layout_click_func);
 
  ui_layout_begin(*lost_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 40.0f));
  ui_layout_push_text(*lost_layout, "Relive", 40.0f, nikola::Vec4(1.0f, 0.0f, 0.0f, 0.0f));
  ui_layout_push_text(*lost_layout, "Give up", 40.0f, nikola::Vec4(0.0f, 1.0f, 0.0f, 0.0f));
  ui_layout_end(*lost_layout);
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// State manager functions

void state_manager_init(nikola::Window* window) {
  // Layouts init

  nikola::ResourceID font_id = nikola::resources_get_id(nikola::RESOURCE_CACHE_ID, "iosevka_bold");

  init_menu_layout(window, font_id);
  init_won_layout(window, font_id);
  init_lost_layout(window, font_id);

  // Current state init 
  s_manager.current_state = &s_manager.entries[STATE_MENU];

  // Listen events
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_change);
}

void state_manager_update() {
  // Update the current state
  ui_layout_update(s_manager.current_state->layout);
}

void state_manager_render_hud() {
  // Render the current state

  ui_text_render_animation(s_manager.current_state->title, UI_TEXT_ANIMATION_FADE_IN, 10.0f);
  ui_layout_render_animation(s_manager.current_state->layout, UI_TEXT_ANIMATION_FADE_IN, 10.0f);
}

/// State manager functions
/// ----------------------------------------------------------------------
