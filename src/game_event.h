#pragma once

/// ----------------------------------------------------------------------
/// GameEventType
enum GameEventType {
  GAME_EVENT_LEVEL_WON,
  GAME_EVENT_LEVEL_LOST,
  GAME_EVENT_COIN_COLLECTED,
  GAME_EVENT_SOUND_PLAYED,
  GAME_EVENT_CHAPTER_CHANGED,

  GAME_EVENTS_MAX = GAME_EVENT_CHAPTER_CHANGED + 1,
};
/// GameEventType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// GameEvent
struct GameEvent {
  GameEventType type; 

  int sound_type;
};
/// GameEvent
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

using OnGameEventFireFunc = bool(*)(const GameEvent& event, void* dispatcher, void* listener);

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// GameEvent functions

void game_event_listen(const GameEventType type, const OnGameEventFireFunc& func, const void* listener = nullptr);

const bool game_event_dispatch(const GameEvent& event, const void* dispatcher = nullptr);

/// GameEvent functions
/// ----------------------------------------------------------------------
