#include "states/state.h"
#include "ui/ui.h"
#include "levels/level.h"
#include "game_event.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// WonState
struct WonState {
  UIText title; 
  UILayout layout; 

  nikola::DynamicArray<nikola::String> lines; 
  nikola::sizei current_line = 0;
};

static WonState s_won;
/// WonState
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_won_layout_click_func(UILayout& layout, UIText& text, void* user_data) {
  switch(layout.current_option) {
    case 0: // Continue
      level_manager_advance();
      break;
  }
}

static void on_state_change(const GameEvent& event, void* dispatcher, void* listener) {
  if(event.state_type != STATE_WON) {
    return;
  }
 
  // Move through the dialogue when the player reaches the end
  //
  // @NOTE: Having this "advance" function here might not be the best, but it works.
  s_won.current_line++; 
  ui_text_set_string(s_won.title, s_won.lines[s_won.current_line - 1]);
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions

static void read_dialogue_file(const nikola::FilePath& txt_path) {
  // Open the file
  nikola::File file; 
  if(!nikola::file_open(&file, txt_path, (int)(nikola::FILE_OPEN_READ))) {
    NIKOLA_LOG_ERROR("Failed to read the dialogue file at \'%s\'", txt_path.c_str());
    return;
  }

  // Read the whole file into the string
  nikola::String dialogue; 
  nikola::file_read_string(file, &dialogue);

  // Fill the lines array
  nikola::String line = ""; 
  for(nikola::sizei i = 0; i < dialogue.size(); i++) {
    if(dialogue[i] != ';') {
      line += dialogue[i];
      continue;
    }

    // Skip the newline after the delimiter (the `;` in this case)
    i++;

    s_won.lines.push_back(line);
    line = "";
  }

  file.close();
  NIKOLA_LOG_DEBUG("Loaded dialogue at \'%s\'", txt_path.c_str());
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Won state functions

void won_state_init(nikola::Window* window, const nikola::ResourceID& font_id) {
  // Title init 
  UITextDesc text_desc = {
    .string = "",

    .font_id   = font_id,
    .font_size = 50.0f,

    .anchor = UI_ANCHOR_TOP_LEFT, 
    .color  = nikola::Vec4(0.6f, 0.6f, 0.0f, 0.0f),
  };
  ui_text_create(&s_won.title, window, text_desc);

  // Layout init
  UILayout* won_layout = &s_won.layout;
  ui_layout_create(won_layout, 
                   window, 
                   font_id,
                   on_won_layout_click_func);
 
  ui_layout_begin(*won_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 0.0f));
  ui_layout_push_text(*won_layout, "Suffer", 40.0f, nikola::Vec4(1.0f, 0.0f, 0.0f, 0.0f));
  ui_layout_end(*won_layout);

  // Dialogue init
  read_dialogue_file("dialogue.txt");
  
  // Listen to events
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_change);
}

void won_state_reset() {
  s_won.title.color.a = 0.0f;
  for(auto& txt : s_won.layout.texts) {
    txt.color.a = 0.0f;
  }
}

void won_state_process_input() {
  ui_layout_update(s_won.layout);
}

void won_state_render() {
  ui_text_render_animation(s_won.title, UI_TEXT_ANIMATION_FADE_IN, 10.0f);
  ui_layout_render_animation(s_won.layout, UI_TEXT_ANIMATION_FADE_IN, 10.0f);
}

/// Won state functions
/// ----------------------------------------------------------------------
