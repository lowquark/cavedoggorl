#ifndef GAME_GAME_HPP
#define GAME_GAME_HPP

#include <game/Engine.hpp>

namespace game {
  // Single player game
  class Game {
    public:
    class HookHandler {
      public:
      virtual ~HookHandler() = default;
      virtual std::unique_ptr<Action> player_action() { return nullptr; }
    };

    Game(HookHandler & hooks, View & view, world::WorldStore & world_store)
      : hooks(hooks)
      , engine_hooks(*this)
      , eng(engine_hooks, world_store) {
      eng.spawn_player(0, view);
    }

    void run() {
      eng.run();
    }

    private:
    class EngineHooks : public Engine::HookHandler {
      Game & game;

      void on_player_turn(Engine::Id player_id) override {
        auto action = game.hooks.player_action();
        if(action) {
          game.eng.complete_turn(*action);
        } else {
          printf("%s: pausing\n", __PRETTY_FUNCTION__);
          game.eng.set_paused(true);
        }
      }
      void on_transfer(Engine::Id eid, world::Id wid, Vec2i pos) override {
      }

      public:
      EngineHooks(Game & game) : game(game) {}
    };
    friend class EngineHooks;

    HookHandler & hooks;
    EngineHooks engine_hooks;
    Engine eng;
  };
}

#endif
