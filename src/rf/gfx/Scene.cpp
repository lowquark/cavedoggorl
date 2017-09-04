
#include "Scene.hpp"

#include <rf/gfx/draw.hpp>

namespace rf {
  namespace gfx {
    extern std::unique_ptr<TilemapShader> tilemap_shader;

    void Scene::set_viewport(Rect2i r) {
      _viewport = r;
      tilemap.tiles.resize(r.size);
    }

    void Scene::set_state(const game::SceneState & state) {
      this->state = state;
    }
    void Scene::add_missile() {
    }

    void Scene::tick() {
    }
    void Scene::draw() const {
      tilemap.tileset_uri = "file://tiles.png";

      for(unsigned int j = 0 ; j < _viewport.size.y ; j ++) {
        for(unsigned int i = 0 ; i < _viewport.size.x ; i ++) {
          auto & tile = tilemap.tiles.get(Vec2u(i, j));
          tile.tileset_index = 86;
          tile.foreground_color = Tilemap::Color(0x77, 0x66, 0x55);
          tile.background_color = Tilemap::Color(0x11, 0x11, 0x22);
        }
      }

      draw::clip(_draw_rect);
      Vec2i pos = _draw_rect.pos + (_draw_rect.size/2) - (_viewport.size*16/2);
      tilemap_shader->draw(tilemap, pos);
      draw::unclip();
    }

    // rounds the given screen position to the nearest grid location
    Vec2i Scene::grid_pos(Vec2i screen_pos) const {
      return Vec2i(screen_pos.x / 16, screen_pos.y / 16) + _viewport.pos;
    }
    // returns the screen position of the center of the given grid location
    Vec2i Scene::screen_pos(Vec2i grid_pos) const {
      Vec2i local_grid_pos = grid_pos - _viewport.pos;
      return Vec2i(local_grid_pos.x * 16 + 16 / 2,
                   local_grid_pos.y * 16 + 16 / 2);
    }
  }
}

