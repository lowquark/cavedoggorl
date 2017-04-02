#ifndef VIEW_HPP
#define VIEW_HPP

#include "draw.hpp"
#include "Map.hpp"
#include "game.hpp"

#include <queue>

namespace gfx {
  class Animation {
    public:
    virtual ~Animation() = default;

    virtual unsigned int type_id() const { return 0; }
    virtual void step() { }
    virtual void start() { }
    virtual void finish() { }
    virtual bool is_finished() const { return true; }
  };

  struct AgentSprite {
    game::Id type_id = 0;
    Vec2f pos;
    Color color;
  };
  struct TileSprite {
    game::Id type_id = 0;
  };

  class View : public game::View {
    Vec2u _window_size;

    std::deque<Animation *> queued_animations;
    std::vector<Animation *> active_animations;

    Map<TileSprite> tile_sprites;
    std::map<game::Id, AgentSprite> agent_sprites;

    void on_world_load(unsigned int tiles_x, unsigned int tiles_y) override;
    void on_tile_load(game::Id type_id, const Vec2i & pos) override;
    void on_agent_load(game::Id agent_id, game::Id type_id, const Vec2i & pos, const Color & color) override;

    void on_agent_move(game::Id agent_id, const Vec2i & from, const Vec2i & to) override;
    void on_agent_death(game::Id agent_id) override;

    void step_animations();
    void draw();

    public:
    const Vec2u & window_size() const {
      return _window_size;
    }
    void window_size(const Vec2u & size) {
      _window_size = size;
    }

    void skip_animations();
    bool are_animations_finished();

    void update();
  };

  void init();
  void deinit();

  void load();
  void unload();
}

#endif
