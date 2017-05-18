#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <memory>
#include <game/view.hpp>
#include <game/core.hpp>
#include <game/parts.hpp>
#include <util/Vec2.hpp>

namespace game {
  class Game {
    public:
    Game() : u(factory) {}

    struct Player {
      unsigned int _id = 0;
      View * _view = nullptr;

      std::string _name = "Unnamed Player";

      public:
      Player() = default;
      Player(unsigned int id, View * view) : _id(id), _view(view) {}

      decltype(_id) id() const { return _id; }

      decltype(_view) view() const { return _view; }

      decltype(_name) name() const { return _name; }
      void set_name(const decltype(_name) & name) { _name = name; }
    };

    enum StepResult {
      TICK_LIMIT,
      PLAYER_TURN,
    };

    Player * get_player(unsigned int id);
    Player * add_player(unsigned int id, View * view);
    void remove_player(unsigned int id);

    void create_new();
    void save(const std::string & name);
    void load_old(const std::string & name);

    StepResult step(unsigned int max_ticks);

    ObjectHandle object_with_turn();
    Player * object_owner(ObjectHandle obj);

    void move_attack(ObjectHandle obj, Vec2i delta);

    private:
    class PartFactory : public BasePartFactory {
      std::map<std::string, Part * (*)(const std::string &)> ctors;
      Part * create(const std::string & data) override;
      void destroy(Part * p) override;
      public:
      PartFactory();
    };

    View * get_view(unsigned int player_id);

    bool visible(Universe & u, ObjectHandle obj, Vec2i pos);
    bool visible(View & view, Vec2i pos);

    void notify_spawn(Universe & u, ObjectHandle obj);
    void notify_despawn(Universe & u, ObjectHandle obj);
    void notify_movement(Universe & u, ObjectHandle obj, Vec2i from, Vec2i to);

    void move_agent(Universe & u, ObjectHandle obj, AgentPart * agent_part, Vec2i delta);
    void wait_agent(Universe & u, ObjectHandle obj, AgentPart * agent_part);
    void ai_turn(Universe & u, ObjectHandle obj);
    void owner_look_at(Universe & u, ObjectHandle obj);
    std::vector<ObjectHandle> objs_with_turn_this_tick();
    void world_tick();

    std::map<unsigned int, std::unique_ptr<Player>> players;
    ObjectHandle current_object;

    PartFactory factory;
    Universe u;
  };
}

#endif
