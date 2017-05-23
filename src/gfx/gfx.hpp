#ifndef VIEW_HPP
#define VIEW_HPP

#include <gfx/draw.hpp>
#include <util/Map.hpp>
#include <game/FOV.hpp>

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
    unsigned int type_id = 0;
    Vec2f pos;
    Color color;
  };
  struct TileSprite {
    unsigned int type_id = 0;
    bool visible = false;
    bool visited = false;
  };

  // An animated version of the game world
  class GridWorld {
    Vec2f _camera_pos;
    Vec2u _tile_size;
    Rect2i _draw_rect;

    std::deque<Animation *> queued_animations;
    std::vector<Animation *> active_animations;

    unsigned int followed_agent_id = 0;

    Map<TileSprite> tile_sprites;
    std::map<unsigned int, AgentSprite> agent_sprites;


    public:
    void clear_sprites();

    // sets the tile size in pixels
    const Vec2u & tile_size() const {
      return _tile_size;
    }
    void set_tile_size(const Vec2u & tile_size) {
      _tile_size = tile_size;
    }

    // sets the grid size in tiles
    // TODO: clear pending tile events
    void set_size(Vec2u size);

    // rounds the given screen position to the nearest grid location
    Vec2i grid_pos(Vec2i screen_pos) const;
    // returns the screen position of the center of the given grid location
    Vec2i screen_pos(Vec2i grid_pos) const;

    // returns a look_str for the object at the given location
    bool look_str(std::string & dst, Vec2i location) const;


    //// events ////

    // creates an agent sprite
    void add_agent(unsigned int agent_id, unsigned int type_id, const Vec2i & pos, const Color & color);
    // removes an agent sprite
    void remove_agent(unsigned int agent_id);

    // moves / animates a previously added agent
    void move_agent(unsigned int agent_id, const Vec2i & from, const Vec2i & to);

    // sets a given tile
    void set_tile(const Vec2i & pos, unsigned int type_id);
    // clears a given tile
    void clear_tile(const Vec2i & pos);


    // Centers the camera on and follows the given agent
    void follow_agent(unsigned int agent_id);

    void set_fov(const FOV & fov);


    // skips animations
    void skip_animations();
    bool are_animations_finished();

    // steps animations
    void tick();


    //// drawing ////

    const Rect2i & draw_rect() const { return _draw_rect; }
    void set_draw_rect(const Rect2i & draw_rect) { _draw_rect = draw_rect; }

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
    void look(Vec2i screen_pos, const std::string & look_str);
    void look_finish();

    void tick();
    void draw();
  };

  class WorldMessageLog {
    struct Item {
      draw::TextBin text_bin;
      Vec2i pos;
    };

    std::deque<Item> items;
    Rect2i _draw_rect;

    public:
    void push(const std::string & message);

    const Rect2i & draw_rect() const { return _draw_rect; }
    void set_draw_rect(const Rect2i & draw_rect) { _draw_rect = draw_rect; }

    void tick();
    void draw();
  };

  void load_font(const char * ttf_path);

  void load();
  void unload();
}

#endif
