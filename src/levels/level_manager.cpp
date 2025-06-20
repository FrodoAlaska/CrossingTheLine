#include "level.h"
#include "game_event.h"
#include "ui/ui.h"
#include "sound_manager.h"
#include "state_manager.h"

#include <nikola/nikola.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

/// ----------------------------------------------------------------------
/// Consts

const nikola::sizei LEVEL_GROUPS_MAX = 4;
const nikola::sizei GROUP_TEXTS_MAX  = 4;

/// Consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// LevelGroup
struct LevelGroup {
  nikola::String name; 

  nikola::DynamicArray<nikola::FilePath> level_paths;
  nikola::sizei current_level   = 0;
  nikola::sizei coins_collected = 0;
};
/// LevelGroup
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// LevelManager 
struct LevelManager {
  Level* current_level = nullptr;

  LevelGroup groups[LEVEL_GROUPS_MAX];
  nikola::sizei current_group = 1;
  
  UIText texts[GROUP_TEXTS_MAX];
  bool can_show_hud = false;
};

static LevelManager s_manager{};
/// LevelManager 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions

static void init_group_ui(nikola::Window* window) {
  // Group names init

  s_manager.groups[0].name = "Hub"; 
  s_manager.groups[1].name = "Chapter 1"; 
  s_manager.groups[2].name = "Chapter 2"; 
  s_manager.groups[3].name = "Chapter 3"; 

  // UI init

  UITextDesc text_desc = {
    .string = "GROUP NAME",

    .font_id   = nikola::resources_get_id(nikola::RESOURCE_CACHE_ID, "iosevka_bold"),
    .font_size = 30.0f,

    .anchor = UI_ANCHOR_TOP_CENTER, 
    .color  = nikola::Vec4(1.0f, 1.0f, 1.0f, 0.0f),
  };
  ui_text_create(&s_manager.texts[0], window, text_desc);

  text_desc.string = "LEVELS COUNT";
  text_desc.offset = nikola::Vec2(0.0f, 35.0f);
  ui_text_create(&s_manager.texts[1], window, text_desc);
  
  text_desc.string = "KEYS COLLECTED COUNT";
  text_desc.offset = nikola::Vec2(0.0f, 65.0f);
  ui_text_create(&s_manager.texts[2], window, text_desc);
  
  text_desc.string = "Press [ENTER] To Start";
  text_desc.color  = nikola::Vec4(0.0f, 1.0f, 0.0f, 0.0f);
  text_desc.offset = nikola::Vec2(0.0f, 100.0f);
  ui_text_create(&s_manager.texts[3], window, text_desc);
}

static nikola::sizei get_index_from_pos(Entity* point) {
  int pos_z = (int)nikola::physics_body_get_position(point->body).z;

  switch(pos_z) {
    case -8:
      return 1;
    case 8:
      return 3;
    case 24:
      return 2;
  }
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_chapter_changed(const GameEvent& event, void* dispatcher, void* listener) {
  if(event.type != GAME_EVENT_CHAPTER_CHANGED) {
    return;
  }

  // Enable the hud for the specific group
  s_manager.can_show_hud = true;
  
  Entity* point_entt  = (Entity*)dispatcher;
  nikola::sizei index = get_index_from_pos(point_entt);
  LevelGroup* group   = &s_manager.groups[index];

  // Set up the UI
  ui_text_set_string(s_manager.texts[0], group->name);
  
  nikola::String levels_count = (std::to_string(group->current_level) + '/' + std::to_string(group->level_paths.size()));
  ui_text_set_string(s_manager.texts[1], levels_count);
 
  nikola::String keys_left = ("Keys: " + std::to_string(group->coins_collected) + '/' + std::to_string(group->level_paths.size()));
  ui_text_set_string(s_manager.texts[2], keys_left);

  if(nikola::input_key_pressed(nikola::KEY_ENTER)) {
    level_unload(s_manager.current_level);
    level_load(s_manager.current_level, group->level_paths[group->current_level]);
    level_reset(s_manager.current_level);
     
    game_event_dispatch(GameEvent{
      .type       = GAME_EVENT_STATE_CHANGED, 
      .state_type = STATE_LEVEL
    });
  }
}

static void on_coin_collected(const GameEvent& event, void* dispatcher, void* listener) {
  if(event.type != GAME_EVENT_COIN_COLLECTED) {
    return;
  }
  
  LevelGroup* group = &s_manager.groups[s_manager.current_group];
  group->coins_collected++;
}

static void on_state_changed(const GameEvent& event, void* dispatcher, void* listener) {
  if(event.state_type != STATE_LEVEL) {
    return;
  }
  
  level_reset(s_manager.current_level);
}

static void level_directory_iterate_func(const nikola::FilePath& base_dir, const nikola::FilePath& current_dir, void* user_data) {
  /* 
   * @NOTE (17/6/2025, Mohamed): 
   *
   * Since all levels have the `C#L#` convention, where `C` stands for "Chapter" and `L` stands 
   * for "Level", we can be sure that the 2 letter or character of the filename is always the number 
   * of the chapter/level group. We can extract that character to determine which group to add this level file to.
   *
  */
  char group_num = nikola::filepath_filename(current_dir)[1];
  
  // Getting the exact integer representation of the character
  int group_index = group_num - '0';

  // Adding the level file to the group
  s_manager.groups[group_index].level_paths.push_back(current_dir);
}

/// Callbacks
/// ----------------------------------------------------------------------


/// ----------------------------------------------------------------------
/// Level manager functions

void level_manager_init(nikola::Window* window) {
  // Level init
  s_manager.current_level = level_create(window);

  // Level groups init
  nikola::FilePath level_dir = nikola::filepath_append(nikola::filesystem_current_path(), "levels");
  nikola::filesystem_directory_iterate(level_dir, level_directory_iterate_func);
  
  // Load the hub level's content
  level_load(s_manager.current_level, s_manager.groups[0].level_paths[0]);

  // Init UI
  init_group_ui(window);

  // Sounds init
  sound_manager_init(s_manager.current_level);

  // Listen to events
  game_event_listen(GAME_EVENT_CHAPTER_CHANGED, on_chapter_changed);
  game_event_listen(GAME_EVENT_COIN_COLLECTED, on_coin_collected);
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_changed);
}

void level_manager_shutdown() {
  sound_manager_shutdown();
  
  level_unload(s_manager.current_level);
  level_destroy(s_manager.current_level);
}

void level_manager_advance() {
  LevelGroup* group = &s_manager.groups[s_manager.current_group];
  level_unload(s_manager.current_level);
  
  // We'll transition to the hub level if the group is out of levels.
  // Otherwise, we can just continue to the next level in the group.
  
  group->current_level++; 
  if(group->current_level >= group->level_paths.size()) {
    level_load(s_manager.current_level, "levels/C0L0.nklvl");
    game_event_dispatch(GameEvent{
      .type       = GAME_EVENT_STATE_CHANGED, 
      .state_type = STATE_HUB
    });
  } 
  else {
    level_load(s_manager.current_level, group->level_paths[group->current_level]);
    game_event_dispatch(GameEvent {
      .type       = GAME_EVENT_STATE_CHANGED, 
      .state_type = STATE_LEVEL 
    });
  }
}

void level_manager_reset() {
  level_reset(s_manager.current_level);
}

void level_manager_update() {
  level_update(s_manager.current_level);
}

void level_manager_render() {
  level_render(s_manager.current_level);
}

void level_manager_render_hud() {
  level_render_hud(s_manager.current_level);

  UITextAnimation anim_type = s_manager.can_show_hud ? UI_TEXT_ANIMATION_FADE_IN : UI_TEXT_ANIMATION_FADE_OUT;
  for(nikola::sizei i = 0; i < GROUP_TEXTS_MAX; i++) {
    ui_text_render_animation(s_manager.texts[i], anim_type, 5.0f);
  }

  s_manager.can_show_hud = false;
}

void level_manager_render_gui() {
  if(!s_manager.current_level->has_editor) {
    return;
  }

  // Level GUI
  level_render_gui(s_manager.current_level);

  // Level select
  nikola::gui_begin_panel("Level select"); 
  nikola::gui_end_panel(); 
}

Level* level_manager_get_current_level() {
  return s_manager.current_level;
}

/// Level manager functions
/// ----------------------------------------------------------------------
