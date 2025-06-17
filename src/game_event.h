#pragma once

/// ----------------------------------------------------------------------
/// GameEventType
enum GameEventType {
  GAME_EVENT_LEVEL_WON,
  GAME_EVENT_LEVEL_LOST,
  GAME_EVENT_COIN_COLLECTED,
  GAME_EVENT_CHAPTER_CHANGED,

  GAME_EVENTS_MAX = GAME_EVENT_CHAPTER_CHANGED + 1,
};
/// GameEventType
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

using OnGameEventFireFunc = bool(*)(const GameEventType type, void* dispatcher, void* listener);

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// GameEvent functions

void game_event_listen(const GameEventType type, const OnGameEventFireFunc& func, const void* listener = nullptr);

const bool game_event_dispatch(const GameEventType type, const void* dispatcher = nullptr);

/// GameEvent functions
/// ----------------------------------------------------------------------
