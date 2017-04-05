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

  // An animated version of the game world
  class GridWorld {
    Vec2f _camera_pos;
    Vec2u _tile_size;
    Rect2i _draw_rect;

    std::deque<Animation *> queued_animations;
    std::vector<Animation *> active_animations;

    Map<TileSprite> tile_sprites;
    std::map<game::Id, AgentSprite> agent_sprites;


    public:
    // sets the tile size in pixels
    const Vec2u & tile_size() const {
      return _tile_size;
    }
    void set_tile_size(const Vec2u & tile_size) {
      _tile_size = tile_size;
    }

    // sets the grid size in tiles
    void set_size(Vec2u size);
    // sets a given tile
    void set_tile(const Vec2i & pos, game::Id type_id);

    // creates an agent sprite
    void add_agent(game::Id agent_id, game::Id type_id, const Vec2i & pos, const Color & color);
    // removes an agent sprite
    void remove_agent(game::Id agent_id);

    // moves / animates a previously added agent
    void move_agent(game::Id agent_id, const Vec2i & from, const Vec2i & to);


    // rounds the given screen position to the nearest grid location
    Vec2i grid_pos(Vec2i screen_pos);
    // returns the screen position of the center of the given grid location
    Vec2i screen_pos(Vec2i grid_pos);


    // returns a look_str for the object at the given location
    bool look_str(std::string & dst, Vec2i location) const;

    // skips animations
    void skip_animations();
    bool are_animations_finished();

    // steps animations
    void tick();


    const Rect2i & draw_rect() const {
      return _draw_rect;
    }
    void set_draw_rect(const Rect2i & draw_rect) {
      _draw_rect = draw_rect;
    }

    // draws clipped within rect
    void draw();
  };

  class HUDOverlay {
    unsigned int look_ease_timer;
    unsigned int look_ease_timer_max = 10;
    bool look_enabled = false;
    Vec2i look_pos;
    std::string look_str;

    draw::TextBin text_bin;

    public:
    HUDOverlay();

    void look(Vec2i screen_pos, const std::string & look_str);
    void look_finish();

    void tick();
    void draw();
  };

  void load_font(const char * ttf_path);

  void load();
  void unload();
}

#endif
