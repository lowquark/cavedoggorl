
#include "Game.hpp"

namespace rf {
  void GameSave::open() {
    close();
  }
  void GameSave::close() {
  }

  Id GameSave::player_level_id() const {
    return 0;
  }
  void GameSave::set_player_level_id(Id id) {
  }

  Id GameSave::player_object_id() const {
    return 0;
  }
  void GameSave::set_player_object_id(Id id) {
  }

  Level GameSave::level(Id id) {
    return Level();
  }
  void GameSave::set_level(Id id, const Level & l) {
  }

  Game::Game(GameSave & gamesave)
    : gamesave(gamesave) {
    player_level_id = gamesave.player_level_id();
    player_object_id = gamesave.player_object_id();

    level = gamesave.level(player_level_id);
  }
  Game::~Game() {
    save();
    clear_events();
  }

  void Game::save() const {
    gamesave.set_player_level_id(player_level_id);
    gamesave.set_player_object_id(player_object_id);

    gamesave.set_level(player_level_id, level);
  }

  void Game::step() {
    // object turn, or step environment
  }
  void Game::step(const PlayerWaitAction & action) {
    step();
  }
  void Game::step(const PlayerMoveAction & action) {
    step();
  }

  SceneState Game::draw(Rect2i roi) {
    return SceneState();
  }

  void Game::handle_events(GameEventVisitor & v) {
    if(events.size()) {
      auto events_copy = std::move(events);
      for(auto & ev : events_copy) {
        ev->visit(v);
      }
      while(events_copy.size()) {
        delete events_copy.front();
        events_copy.pop_front();
      }
    }
  }
  void Game::clear_events() {
    while(events.size()) {
      delete events.front();
      events.pop_front();
    }
  }

  bool Game::is_player_turn() const {
    return true;
  }
}

