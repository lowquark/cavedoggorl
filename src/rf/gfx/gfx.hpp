#ifndef RF_GFX_GFX_HPP
#define RF_GFX_GFX_HPP

#include <rf/util/Map.hpp>
#include <rf/game/FOV.hpp>
#include <rf/gfx/draw.hpp>
#include <rf/gfx/TileMap.hpp>

#include <deque>

namespace rf {
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
      public:
      void clear_sprites();

      Vec2i camera_pos() const { return _camera_rect.pos; }
      void set_camera_pos(const Vec2i & pos) { _camera_rect.pos = pos; }
      Vec2i camera_size() const { return _camera_rect.size; }
      void set_camera_size(const Vec2i & size) { _camera_rect.size = size; }
      int camera_margin() const { return _camera_margin; }
      void set_camera_margin(int margin) { _camera_margin = margin; }

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

      // sets a given tile
      void set_tile(const Vec2i & pos, unsigned int type_id);
      // clears a given tile
      void clear_tile(const Vec2i & pos);


      // Centers the camera on and follows the given agent
      void follow_agent(unsigned int agent_id);

      void set_fov(const rf::FOV & fov);


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

      void update_camera(Vec2i focus, int margin);

      private:
      Rect2i _draw_rect;

      Rect2i _camera_rect;
      int _camera_margin = 0;
      unsigned int followed_agent_id = 0;


      draw::TileMap tilemap; // tilemap of sprites displayed on screen
      Map<TileSprite> tile_sprites; // sprites of the world
      std::map<unsigned int, AgentSprite> agent_sprites;


      std::deque<Animation *> queued_animations;
      std::vector<Animation *> active_animations;
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
    bool load_tiles(const char * png_path, Vec2u tile_size);

    void load();
    void unload();
  }
}

#endif
