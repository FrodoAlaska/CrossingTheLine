#include "level.h"
#include "states/state.h"
#include "game_event.h"
#include "ui/ui.h"
#include "sound_manager.h"
#include "input_manager.h"

#include <nikola/nikola.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

/// ----------------------------------------------------------------------
/// Consts

const nikola::sizei GROUP_TEXTS_MAX  = 4;

/// Consts
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// LevelGroup
struct LevelGroup {
  nikola::String name; 
  nikola::sizei index;

  nikola::DynamicArray<nikola::FilePath> level_paths;
  nikola::sizei current_level   = 0;
  nikola::sizei coins_collected = 0;

  bool is_locked;
};
/// LevelGroup
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// LevelManager 
struct LevelManager {
  Level* current_level = nullptr;

  LevelGroup groups[LEVEL_GROUPS_MAX];
  LevelGroup* selected_group  = nullptr;
  nikola::sizei current_group = 1;
  
  UIText texts[GROUP_TEXTS_MAX];
  bool can_show_hud = false;
};

static LevelManager s_manager{};
/// LevelManager 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions

static void init_group_ui(nikola::Window* window, const nikola::ResourceID& font_id) {
  // Groups init

  s_manager.groups[0].name      = "Hub"; 
  s_manager.groups[0].index     = 0;
  s_manager.groups[0].is_locked = false;

  s_manager.groups[1].name      = "Chapter 1"; 
  s_manager.groups[1].index     = 1;
  s_manager.groups[1].is_locked = false;
  
  s_manager.groups[2].name      = "Chapter 2"; 
  s_manager.groups[2].index     = 2;
  s_manager.groups[2].is_locked = true;
  
  s_manager.groups[3].name      = "Chapter 3"; 
  s_manager.groups[3].index     = 3;
  s_manager.groups[3].is_locked = true;
  
  s_manager.groups[4].name      = "Chapter 4"; 
  s_manager.groups[4].index     = 4;
  s_manager.groups[4].is_locked = true;

  // UI init

  UITextDesc text_desc = {
    .string = "GROUP NAME",

    .font_id   = font_id,
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
  int pos_x = (int)nikola::physics_body_get_position(point->body).x;
  
  switch(pos_x) {
    case -8:
      return 1;
    case 0:
      return 2;
    case 8:
      return 3;
    case 24:
      return 4;
  }
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_chapter_changed(const GameEvent& event, void* dispatcher, void* listener) {
  if(event.type == GAME_EVENT_CHAPTER_EXITED) {
    s_manager.selected_group = nullptr;
    return;
  }

  Entity* point_entt       = (Entity*)dispatcher;
  s_manager.selected_group = &s_manager.groups[get_index_from_pos(point_entt)];
  LevelGroup* group        = s_manager.selected_group;

  // Set up the UI
  ui_text_set_string(s_manager.texts[0], group->name);
  
  nikola::String levels_count = ("Levels: " + std::to_string(group->level_paths.size()));
  ui_text_set_string(s_manager.texts[1], levels_count);
 
  nikola::String keys_left = ("Keys: " + std::to_string(group->coins_collected) + '/' + std::to_string(group->level_paths.size()));
  ui_text_set_string(s_manager.texts[2], keys_left);
  
  nikola::String continue_str = "Start your journey";
  nikola::Vec4 text_color     = nikola::Vec4(0.0f, 1.0f, 0.0f, s_manager.texts[3].color.a);
  
  if(group->is_locked) {
    continue_str = "You're still too weak..."; 
    text_color   = nikola::Vec4(1.0f, 0.0f, 0.0f, text_color.a);
  }

  s_manager.texts[3].color = text_color;
  ui_text_set_string(s_manager.texts[3], continue_str);
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

  // Reset the level before switching
  level_reset(s_manager.current_level);
 
  // Play a specific sound depending on the level being loaded
  nikola::FilePath lvl_name = nikola::filepath_filename(s_manager.current_level->nkbin.path); 
  
  // Cool sound event
  GameEvent sound_event = {
    .type       = GAME_EVENT_MUSIC_PLAYED, 
    .sound_type = (lvl_name == "C0L0.nklvl") ? SOUND_HUB : SOUND_AMBIANCE, // @TODO: There's probably a better way to do this
  };
  game_event_dispatch(sound_event);
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

void level_manager_init(nikola::Window* window, const nikola::ResourceID& font_id) {
  // Level init
  s_manager.current_level = level_create(window);

  // Level groups init
  nikola::FilePath level_dir = nikola::filepath_append(nikola::filesystem_current_path(), "levels");
  nikola::filesystem_directory_iterate(level_dir, level_directory_iterate_func);
  
  // Load the hub level's content
  level_load(s_manager.current_level, s_manager.groups[0].level_paths[0]);

  // Init UI
  init_group_ui(window, font_id);

  // Listen to events
  game_event_listen(GAME_EVENT_CHAPTER_ENTERED, on_chapter_changed);
  game_event_listen(GAME_EVENT_CHAPTER_EXITED, on_chapter_changed);
  game_event_listen(GAME_EVENT_COIN_COLLECTED, on_coin_collected);
  game_event_listen(GAME_EVENT_STATE_CHANGED, on_state_changed);
}

void level_manager_shutdown() {
  sound_manager_shutdown();
  
  level_unload(s_manager.current_level);
  level_destroy(s_manager.current_level);
}

void level_manager_reset() {
  nikola::u8 current_group, coins_collected;
  nkdata_file_get_level_data(&current_group, &coins_collected);

  // Loading the settings of the current group
  
  s_manager.current_group = current_group;

  s_manager.groups[s_manager.current_group].coins_collected = coins_collected;
  s_manager.groups[s_manager.current_group].is_locked       = false;

  // We are assuming that all of the previous levels 
  // before the current group were completed, since 
  // every group HAS to be completed before advancing to 
  // the next group.
  //
  // Probably not the best idea, but it works.
  for(nikola::sizei i = 0; i < s_manager.current_group; i++) {
    LevelGroup* group = &s_manager.groups[i];

    group->coins_collected = group->level_paths.size();
    group->is_locked       = false;
  }

  // Load the hub level
  level_unload(s_manager.current_level);
  level_load(s_manager.current_level, s_manager.groups[0].level_paths[0]);
}

void level_manager_advance() {
  LevelGroup* level_group = &s_manager.groups[s_manager.current_group];
  level_unload(s_manager.current_level);
  
  // We'll transition to the hub level if the group is out of levels.
  // Otherwise, we can just continue to the next level in the group.
  
  level_group->current_level++; 
  if(level_group->current_level < level_group->level_paths.size()) {
    level_load(s_manager.current_level, level_group->level_paths[level_group->current_level]);
    game_event_dispatch(GameEvent {
      .type       = GAME_EVENT_STATE_CHANGED, 
      .state_type = STATE_LEVEL 
    });

    nkdata_file_set_level_data(s_manager.current_group, level_group->coins_collected);
    return;
  } 

  // We're out of groups...
  s_manager.current_group++; 
  if(s_manager.current_group >= LEVEL_GROUPS_MAX) {
    level_load(s_manager.current_level, s_manager.groups[0].level_paths[0]);
    game_event_dispatch(GameEvent{
      .type       = GAME_EVENT_STATE_CHANGED, 
      .state_type = STATE_CREDITS
     });

    return;
  }
  
  // We cannot advance into the next group of levels until 
  // the player has collected all of the coins of the previous group. 

  LevelGroup* prev_group = level_group; 
  level_group            = &s_manager.groups[s_manager.current_group];
  level_group->is_locked = (prev_group->coins_collected < prev_group->level_paths.size());
  
  // Save the current state
  nkdata_file_set_level_data(s_manager.current_group, level_group->coins_collected);
 
  // To the hub world!
  level_load(s_manager.current_level, s_manager.groups[0].level_paths[0]);
  game_event_dispatch(GameEvent{
    .type       = GAME_EVENT_STATE_CHANGED, 
    .state_type = STATE_LEVEL
  });
}

void level_manager_process_input() {
  // Level input
  level_process_input(s_manager.current_level);
  
  if(!input_manager_action_pressed(INPUT_ACTION_ACCEPT)) {
    return;
  }

  LevelGroup* group = s_manager.selected_group;
  if(!group) {
    return;
  }

  if(group->is_locked) {
    game_event_dispatch(GameEvent{
      .type       = GAME_EVENT_SOUND_PLAYED, 
      .sound_type = SOUND_FAIL_INPUT,
    });
  }
  else {
    // Reset the group's variables
    group->current_level    = 0;
    group->coins_collected  = 0;
    s_manager.current_group = group->index;

    // Loading the new level
    level_unload(s_manager.current_level);
    level_load(s_manager.current_level, group->level_paths[group->current_level]);
     
    game_event_dispatch(GameEvent{
      .type       = GAME_EVENT_STATE_CHANGED, 
      .state_type = STATE_LEVEL
    });

    s_manager.selected_group = nullptr;
  }
}

void level_manager_update() {
  level_update(s_manager.current_level);
}

void level_manager_render() {
  level_render(s_manager.current_level);
}

void level_manager_render_hud() {
  UITextAnimation anim_type = s_manager.selected_group ? UI_TEXT_ANIMATION_FADE_IN : UI_TEXT_ANIMATION_FADE_OUT;
  for(nikola::sizei i = 0; i < GROUP_TEXTS_MAX; i++) {
    ui_text_render_animation(s_manager.texts[i], anim_type, 5.0f);
  }

  level_render_hud(s_manager.current_level);
}

void level_manager_render_gui() {
  if(!s_manager.current_level->has_editor) {
    return;
  }

  // Level GUI
  level_render_gui(s_manager.current_level);

  // Level select
  nikola::gui_begin_panel("Level select"); 
  for(nikola::sizei i = 0; i < LEVEL_GROUPS_MAX; i++) {
    if(!ImGui::CollapsingHeader(s_manager.groups[i].name.c_str())) {
      continue;
    } 

    static nikola::String current_level_name = "C0L0.nklvl";
    if(!ImGui::BeginCombo("Level", current_level_name.c_str())) {
      continue; 
    }

    for(auto& path : s_manager.groups[i].level_paths) {
      if(ImGui::Selectable(nikola::filepath_filename(path).c_str())) {
        level_unload(s_manager.current_level);
        if(!level_load(s_manager.current_level, path)) {
          continue;
        }
        
        level_reset(s_manager.current_level);
        current_level_name = nikola::filepath_filename(path); 
      }
    }
     
    ImGui::EndCombo();
  }
  nikola::gui_end_panel(); 
}

void level_manager_get_current_indices(nikola::sizei* group_index, nikola::sizei* level_index) {
  *group_index = s_manager.current_group;
  *level_index = s_manager.groups[s_manager.current_group].current_level;
}

Level* level_manager_get_current_level() {
  return s_manager.current_level;
}

/// Level manager functions
/// ----------------------------------------------------------------------
