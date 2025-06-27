#include "states/state.h"
#include "ui/ui.h"
#include "levels/level.h"
#include "game_event.h"
#include "input_manager.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// WonState
struct WonState {
  UIText title; 
  UILayout layout; 

  nikola::DynamicArray<nikola::String> lines; 
  nikola::sizei current_line = 0;

  nikola::Timer animation_timer;
  nikola::sizei total_characters = 0;
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

static void skip_comment(nikola::sizei* index, const nikola::String& source) {
  char ch = source[*index];
  while(ch != '\n') {
    *index++;
    ch = source[*index];
  }
}

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
    switch(dialogue[i]) {
      case '\n':
        break; 
      case '#':
        i++;
        skip_comment(&i, dialogue);
        break; 
      case ';':
        s_won.lines.push_back(line);
        line = "";
        break;
      default:
        line += dialogue[i];
        break;
    } 
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
    .font_size = 40.0f,

    .anchor = UI_ANCHOR_TOP_LEFT, 
    .color  = nikola::Vec4(1.0f),
  };
  ui_text_create(&s_won.title, window, text_desc);

  // Layout init
  UILayout* won_layout = &s_won.layout;
  ui_layout_create(won_layout, 
                   window, 
                   font_id,
                   on_won_layout_click_func);
  won_layout->is_active = false;

  ui_layout_begin(*won_layout, UI_ANCHOR_CENTER, nikola::Vec2(0.0f, 0.0f));
  ui_layout_push_text(*won_layout, "Suffer", 40.0f, nikola::Vec4(1.0f, 0.0f, 0.0f, 0.0f));
  ui_layout_end(*won_layout);

  // Dialogue init
  read_dialogue_file("dialogue.txt");
  
  // Listen to events
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_change);

  // Timer init
  nikola::timer_create(&s_won.animation_timer, 8.0f, false);
}

void won_state_reset() {
  for(auto& txt : s_won.layout.texts) {
    txt.color.a = 0.0f;
  }

  s_won.total_characters = 0;
  s_won.layout.is_active = false;
}

void won_state_process_input() {
  nikola::timer_update(s_won.animation_timer);
  ui_layout_update(s_won.layout);

  if(input_manager_action_pressed(INPUT_ACTION_ACCEPT)) {
    s_won.layout.is_active = true; 
    s_won.total_characters = s_won.title.string.size();
  }
}

void won_state_render() {
  // Get the window bounds
  int width, height; 
  nikola::window_get_size(s_won.title.window_ref, &width, &height);

  // Render the layout
  ui_layout_render_animation(s_won.layout, UI_TEXT_ANIMATION_BLINK, 8.0f);

  // Render the next character if the timer runs out
  if(s_won.animation_timer.has_runout) {
    s_won.total_characters++;

    if(s_won.total_characters >= s_won.title.string.size()) {
      s_won.total_characters = s_won.title.string.size(); 
      s_won.layout.is_active = true;
    }
  }
  
  // Render the dialogue

  nikola::Vec2 off   = nikola::Vec2(0.0f);
  nikola::Vec2 pos   = nikola::Vec2(20.0f, s_won.title.font_size + 10.0f);
  float scale        = s_won.title.font_size / 256.0f;
  float prev_advance = 0.0f;

  float wrap_limit = width - s_won.title.font_size - 20.0f;

  for(nikola::sizei i = 0; i < s_won.total_characters; i++) {
    char ch             = s_won.title.string[i];
    nikola::Glyph glyph = s_won.title.font->glyphs[ch];
    
    if(ch == '\n') {
      off.x = 0.0f;
      off.y += s_won.title.font_size + 2.0f;

      continue;
    }
    else if(ch == ' ' || ch == '\t') {
      off.x += prev_advance * scale;
      continue;
    }
    
    off.x       += glyph.advance_x * scale;
    prev_advance = glyph.advance_x;

    if((off.x + pos.x) >= wrap_limit) {
      off.y += s_won.title.font_size + 2.0f;
      off.x  = pos.x;
    }
    
    nikola::batch_render_codepoint(s_won.title.font, ch, pos + off, s_won.title.font_size, s_won.title.color); 
  } 
}

/// Won state functions
/// ----------------------------------------------------------------------
