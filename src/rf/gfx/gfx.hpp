#ifndef RF_GFX_GFX_HPP
#define RF_GFX_GFX_HPP

#include <rf/util/Map.hpp>
#include <rf/game/Game.hpp>
#include <rf/gfx/draw.hpp>
#include <rf/gfx/Tilemap.hpp>

#include <deque>

namespace rf {
  namespace gfx {
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
    void load_tiles(const char * png_path);

    void load();
    void unload();
  }
}

#endif
