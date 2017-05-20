#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <memory>
#include <game/view.hpp>
#include <game/core.hpp>
#include <util/Vec2.hpp>

namespace game {
  struct AgentPart;

  class World {
    public:
    World() : uni(factory) {}

    // returns a valid object handle if a turn has been reached before
    // `max_ticks` # of ticks have been exceeded
    ObjectHandle tick_until_turn(unsigned int max_ticks);

    // returns true, and the controller id if the entity is player controlled.
    // returns false and an unspecified controller id otherwise
    std::pair<bool, unsigned int> is_player_controlled(ObjectHandle obj) const;
    // executes an automatic turn, defaults to wait in the case of no AI
    void ai_turn(ObjectHandle obj);

    void wait_turn(ObjectHandle obj);
    // returns false if there is an obstacle in the way, and it is not
    // destructible
    bool move_attack_turn(ObjectHandle obj, Vec2i delta);

    // adds a global view, updates to current state
    void add_view(View * view);
    // adds a view with a particular perspective, updates to current state
    void add_view(View * view, ObjectHandle obj);

    void set_size(unsigned int w, unsigned int h);
    void set_tile(Vec2i pos, unsigned int id);

    ObjectHandle create_hero(Vec2i pos);
    ObjectHandle create_badguy(Vec2i pos);

    private:
    class PartFactory : public BasePartFactory {
      std::map<std::string, Part * (*)(const std::string &)> ctors;
      Part * create(const std::string & data) override;
      void destroy(Part * p) override;
      public:
      PartFactory();
    };

    struct ObjectView {
      View * view = nullptr;
      ObjectHandle object;

      bool visible(Vec2i pos) const { return true; }
    };

    void update_view(ObjectView view);
    void update_view(View * view);

    void notify_spawn(ObjectHandle obj);
    void notify_despawn(ObjectHandle obj);
    void notify_move(ObjectHandle obj, Vec2i from, Vec2i to);

    // the world does all the things
    std::vector<ObjectHandle> objs_with_turn_this_tick() const;
    void tick();

    PartFactory factory;
    Universe uni;
    Map<unsigned int> tiles;

    // see everything
    std::vector<View *> global_views;
    // only see what their associated object sees
    std::vector<ObjectView> object_views;

    // Its big, it's heavy, it's wood
    Log log;
    // Better than bad, it's good!
  };
}

#endif
