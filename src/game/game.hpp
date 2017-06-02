#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <memory>
#include <game/view.hpp>
#include <game/core.hpp>
#include <util/Vec2.hpp>

namespace game {
  struct Space {
    Map<unsigned int> tiles;
    Map<unsigned int> opaque;
    std::vector<ObjectHandle> agents;
    std::vector<ObjectHandle> items;
  };

  class WorldResizeHandler {
    public:
    virtual void on_world_resize(unsigned int w, unsigned int h) = 0;
  };
  class TileUpdateHandler {
    public:
    virtual void on_tile_update(Vec2i pos, unsigned int new_val) = 0;
  };
  class SpawnHandler {
    public:
    virtual void on_spawn(ObjectHandle obj) = 0;
  };
  class DespawnHandler {
    public:
    virtual void on_despawn(ObjectHandle obj) = 0;
  };
  class MoveHandler {
    public:
    virtual void on_move(ObjectHandle obj, Vec2i from, Vec2i to) = 0;
  };
  class FOVUpdateHandler {
    public:
    virtual void on_fov_update(ObjectHandle obj) = 0;
  };

  // Systems exist to split up the code in World
  class ViewSys : public WorldResizeHandler,
                  public TileUpdateHandler,
                  public SpawnHandler,
                  public DespawnHandler,
                  public MoveHandler,
                  public FOVUpdateHandler {
    public:
    ViewSys(View & view, Universe & uni, Space & space) : view(view), uni(uni), space(space) {}

    void on_world_resize(unsigned int w, unsigned int h) override;
    void on_tile_update(Vec2i pos, unsigned int new_val) override;

    void on_spawn(ObjectHandle obj) override;
    void on_despawn(ObjectHandle obj) override;
    void on_move(ObjectHandle obj, Vec2i from, Vec2i to) override;

    void on_fov_update(ObjectHandle obj) override;

    private:
    View & view;
    Universe & uni;
    Space & space;
  };

  class FOVSys : public SpawnHandler, public MoveHandler {
    public:
    typedef std::vector<FOVUpdateHandler *> FOVUpdateHandlerList;

    FOVSys(Universe & uni, const Space & space, const FOVUpdateHandlerList & fov_update_handlers)
      : uni(uni)
      , space(space)
      , fov_update_handlers(fov_update_handlers)
    {}

    void on_spawn(ObjectHandle obj) override;
    void on_move(ObjectHandle obj, Vec2i from, Vec2i to) override;

    private:
    Universe & uni;
    const Space & space;
    FOVUpdateHandlerList fov_update_handlers;
  };
  class DijkstraSys : public MoveHandler {
    public:
    DijkstraSys(Universe & uni)
      : uni(uni)
    {}

    void on_move(ObjectHandle obj, Vec2i from, Vec2i to) override {
      printf("%s\n", __PRETTY_FUNCTION__);
    }

    private:
    Universe & uni;
  };
  class PhysicsSys {
    public:
    typedef std::vector<MoveHandler *> MoveHandlerList;

    PhysicsSys(Universe & uni, Space & space, const MoveHandlerList & move_handlers)
      : uni(uni)
      , space(space)
      , move_handlers(move_handlers)
    {}

    bool can_move(ObjectHandle obj, Vec2i delta) {
      printf("%s\n", __PRETTY_FUNCTION__);
      return true;
    }
    unsigned int move(ObjectHandle obj, Vec2i delta);
    unsigned int hit(ObjectHandle obj, Vec2i delta) {
      printf("%s\n", __PRETTY_FUNCTION__);
      return 10;
    }

    bool is_passable(Vec2i pos);

    std::vector<Vec2i> find_path(Vec2i from, Vec2i to);

    private:
    Universe & uni;
    Space & space;

    MoveHandlerList move_handlers;

    void notify_move(ObjectHandle obj, Vec2i from, Vec2i to);
  };
  class AISys {
    public:
    AISys(Universe & uni, PhysicsSys & phys_sys, DijkstraSys & dijkstra_sys)
      : uni(uni)
      , phys_sys(phys_sys)
      , dijkstra_sys(dijkstra_sys) {}

    unsigned int on_turn(ObjectHandle obj);

    private:
    Universe & uni;
    PhysicsSys & phys_sys;
    DijkstraSys & dijkstra_sys;
  };

  class EntityManager : public newcore::EntityManager {
    public:
    EntityManager();

    private:
    std::map<std::string, newcore::Part * (*)(const std::string &)> part_ctors;

    newcore::Part * create_part(newcore::EntityId id, const std::string & data) override;
    void destroy_part(newcore::EntityId id, newcore::Part * p) override;
  };

  class Action;

  class Engine {
    public:
    Engine(View & view)
      : uni(factory)
      , ai_sys(uni, phys_sys, dijkstra_sys)
      , phys_sys(uni, space, { &view_sys, &fov_sys, &dijkstra_sys })
      , fov_sys(uni, space, { &view_sys })
      , dijkstra_sys(uni)
      , view_sys(view, uni, space)
    {}

    std::pair<bool, ObjectHandle> step(unsigned int max_ticks);
    void complete_turn(const Action & action);

    void move_attack(ObjectHandle obj, Vec2i delta);
    void wait(ObjectHandle obj);

    //void set_size(unsigned int w, unsigned int h);
    //void set_tile(Vec2i pos, unsigned int id);

    unsigned int create_space(Vec2u size);
    void         destroy_space(unsigned int sid);

    ObjectHandle create_hero();
    ObjectHandle create_badguy(ObjectHandle kill_obj);
    void spawn(ObjectHandle obj, unsigned int sid, Vec2i pos);

    private:
    class PartFactory : public BasePartFactory {
      std::map<std::string, Part * (*)(const std::string &)> ctors;
      public:
      PartFactory();
      virtual Part * create(const std::string & data) override;
      virtual void destroy(Part * p) override;
    };

    EntityManager entities;

    PartFactory factory;
    Universe uni;
    Space space;
    std::map<unsigned int, Space> spaces;

    ObjectHandle incomplete_turn;

    AISys ai_sys;
    PhysicsSys phys_sys;
    FOVSys fov_sys;
    DijkstraSys dijkstra_sys;
    ViewSys view_sys;

    ObjectHandle get_player();

    std::vector<ObjectHandle> objs_with_turn_this_tick() const;

    void tick();

    // Its big, it's heavy, it's wood
    Log log;
    // Better than bad, it's good!
  };

  class Action {
    public:
    virtual ~Action() = default;
    virtual void perform(Engine & E, ObjectHandle obj) const = 0;
  };

  class MoveAction : public Action {
    Vec2i delta;
    public:
    MoveAction(Vec2i delta) : delta(delta) {}
    void perform(Engine & E, ObjectHandle obj) const override;
  };

  class WaitAction : public Action {
    public:
    void perform(Engine & E, ObjectHandle obj) const override;
  };
}

#endif
