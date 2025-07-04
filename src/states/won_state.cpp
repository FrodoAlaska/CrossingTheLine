#include "states/state.h"
#include "ui/ui.h"
#include "levels/level.h"
#include "game_event.h"
#include "input_manager.h"
#include "sound_manager.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// WonState
struct WonState {
  UIText title; 
  UILayout layout; 

  nikola::DynamicArray<nikola::String> lines[LEVEL_GROUPS_MAX]; 
  nikola::sizei current_line = 0;

  nikola::Timer animation_timer;
  nikola::sizei total_characters = 0;

  nikola::sizei char_limit = 0; 
  float wrap_limit         = 0.0f;
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

  GameEvent sound_event = {
    .type       = GAME_EVENT_MUSIC_PLAYED, 
    .sound_type = SOUND_HUB,
  };
  game_event_dispatch(sound_event);
  
  // Move through the dialogue when the player reaches the end

  nikola::sizei group_index, level_index;
  level_manager_get_current_indices(&group_index, &level_index);

  ui_text_set_string(s_won.title, s_won.lines[group_index][level_index]);
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions

static void add_string_literal(nikola::sizei* index, const nikola::String& source, const nikola::sizei current_group) {
  nikola::sizei line_width = 0;
  nikola::String line;

  char ch = source[*index];
  while(ch != '"') {
    // Ignore new line characters.
    if(ch != '\n') {
      line.push_back(ch); 
      line_width++;
    }

    // Advance
    *index += 1;
    ch = source[*index];

    // The checks below only apply when there's a new ' ' character encountered
    if(ch != ' ') {
      continue;
    }

    // The line has been fed enough. We should go to the next line.
    if(line_width >= s_won.char_limit) {
      line.push_back('\n');
      line_width = 0;
   
      // Skip the space since it's going to be a new line
      *index += 1;
      ch = source[*index];
    }
  }

  s_won.lines[current_group].push_back(line);
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
  
  nikola::sizei current_group = 0;

  for(nikola::sizei i = 0; i < dialogue.size(); i++) {
    switch(dialogue[i]) {
      case '#':
        current_group++;
        break; 
      case '"':
        i++;
        add_string_literal(&i, dialogue, current_group);
        break;
      default:
        break;
    } 
  }

  nikola::file_close(file);
  NIKOLA_LOG_INFO("Loaded dialogue at \'%s\'", txt_path.c_str());
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

  // Variables init
  
  int width, height; 
  nikola::window_get_size(s_won.title.window_ref, &width, &height);

  /* @NOTE (2/7/2025, Mohamed):
   * 
   * The `wrap_limit` is exactly what it sounds. Characters should not go beyond this 
   * limit. We retrieve this limit dynamically by taking the width of the current window 
   * and subtract an arbitrary value like the font size * 2. 
   *
   * As for `char_limit` is takes into account the current wrap limit to deduce how many 
   * characters, theoretically, will be on a single line. Since we can't really know _exactly_ 
   * how many characters are going to be in a single line (different sizes, multiple space, etc), 
   * we try to approximate by giving it a completely magical number `16.6`. 
   *
   * According to my tests, this works on multiple screen resolutions. However, it will only 
   * work based on the _current_ font size of the title string. I haven't tested it, but I'm 
   * guessing that if the font size changes that magical number isn't going to be _magical_ anymore.
   *
   * Very hacky. Made with glue and tape. But it works nonetheless.
   *
   */
  s_won.wrap_limit = width - s_won.title.font_size * 2.0f;
  s_won.char_limit = (nikola::sizei)(s_won.wrap_limit / 16.6f); // @TODO: Explain

  // Dialogue init
  read_dialogue_file("res/dialogue.txt");
  
  // Listen to events
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_change);

  // Timer init
  nikola::timer_create(&s_won.animation_timer, 6.0f, false);
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

  nikola::Vec2 off     = nikola::Vec2(0.0f);
  nikola::Vec2 padding = nikola::Vec2(20.0f); 
  nikola::Vec2 pos     = nikola::Vec2(padding.x, s_won.title.font_size + padding.y);

  float scale        = s_won.title.font_size / 256.0f;
  float prev_advance = 0.0f;

  for(nikola::sizei i = 0; i < s_won.total_characters; i++) {
    char ch             = s_won.title.string[i];
    nikola::Glyph glyph = s_won.title.font->glyphs[ch];
     
    // Take into account the new line
    if(ch == '\n') {
      off.x  = 0.0f;
      off.y += s_won.title.font_size + 2.0f;

      continue;
    }
    // Take into account the spaces
    else if(ch == ' ' || ch == '\t') {
      off.x += prev_advance * scale;
      continue;
    }
   
    // Apply the offsets
    off.x += glyph.advance_x * scale;
   
    // Render the character
    nikola::batch_render_codepoint(s_won.title.font, ch, pos + off, s_won.title.font_size, s_won.title.color);
  
    // Resetting some variables 
    prev_advance = glyph.advance_x;
  } 
}

/// Won state functions
/// ----------------------------------------------------------------------
