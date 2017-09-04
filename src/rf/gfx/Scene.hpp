#ifndef RF_GFX_SCENE_HPP
#define RF_GFX_SCENE_HPP

#include <rf/game/Game.hpp>
#include <rf/gfx/Tilemap.hpp>

namespace rf {
  namespace gfx {
    class Scene {
      public:
      Rect2i viewport() const { return _viewport; }
      void set_viewport(Rect2i r);

      void set_tileset(const std::string & uri);
      void set_state(const game::SceneState & state);
      void add_missile();

      void tick();
      void draw() const;

      // rounds the given screen position to the nearest grid location
      Vec2i grid_pos(Vec2i screen_pos) const;
      // returns the screen position of the center of the given grid location
      Vec2i screen_pos(Vec2i grid_pos) const;

      const Rect2i & draw_rect() const { return _draw_rect; }
      void set_draw_rect(const Rect2i & draw_rect) { _draw_rect = draw_rect; }

      private:
      // on-screen rect to draw within
      Rect2i _draw_rect;
      // in-game rect to sample from
      Rect2i _viewport;

      // updated when set_tileset is called
      Vec2u tile_size;

      game::SceneState state;

      mutable Tilemap tilemap;
    };
  }
}

#endif
