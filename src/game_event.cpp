#include "game_event.h"

#include <nikola/nikola_containers.h>

/// ----------------------------------------------------------------------
/// GameEventEntry 
struct GameEventEntry {
  OnGameEventFireFunc func; 
  void* listener;
};
/// GameEventEntry 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// GameEventPool
struct GameEventPool {
  nikola::DynamicArray<GameEventEntry> events[GAME_EVENTS_MAX];
};

static GameEventPool s_pool;
/// GameEventPool
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// GameEvent functions

void game_event_listen(const GameEventType type, const OnGameEventFireFunc& func, const void* listener) {
  s_pool.events[type].push_back(GameEventEntry{});
  GameEventEntry* entry = &s_pool.events[type][s_pool.events[type].size() - 1];

  NIKOLA_ASSERT(func, "Cannot listen to an event with an invalid callback");
  entry->func     = func; 
  entry->listener = (void*)listener;
}

void game_event_dispatch(const GameEvent& event, const void* dispatcher) {
  for(nikola::sizei i = 0; i < s_pool.events[event.type].size(); i++) {
    GameEventEntry* entry = &s_pool.events[event.type][i];
    entry->func(event, (void*)dispatcher, entry->listener);
  }
}

/// GameEvent functions
/// ----------------------------------------------------------------------
