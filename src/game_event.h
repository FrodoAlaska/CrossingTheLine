#pragma once

/// ----------------------------------------------------------------------
/// GameEventType
enum GameEventType {
  GAME_EVENT_STATE_CHANGED,
  GAME_EVENT_COIN_COLLECTED,
  GAME_EVENT_SOUND_PLAYED,
  GAME_EVENT_MUSIC_PLAYED,
  GAME_EVENT_CHAPTER_ENTERED,
  GAME_EVENT_CHAPTER_EXITED,

  GAME_EVENTS_MAX = GAME_EVENT_CHAPTER_EXITED + 1,
};
/// GameEventType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// GameEvent
struct GameEvent {
  GameEventType type; 

  int sound_type;
  int state_type;
};
/// GameEvent
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

using OnGameEventFireFunc = void(*)(const GameEvent& event, void* dispatcher, void* listener);

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// GameEvent functions

void game_event_listen(const GameEventType type, const OnGameEventFireFunc& func, const void* listener = nullptr);

void game_event_dispatch(const GameEvent& event, const void* dispatcher = nullptr);

/// GameEvent functions
/// ----------------------------------------------------------------------
