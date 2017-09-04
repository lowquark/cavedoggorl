
#include "Scene.hpp"

#include <rf/gfx/draw.hpp>

namespace rf {
  namespace gfx {
    extern std::unique_ptr<TilemapShader> tilemap_shader;
    extern Vec2u get_tileset_tile_size(const std::string & uri);

    void Scene::set_viewport(Rect2i r) {
      _viewport = r;
      tilemap.tiles.resize(r.size);
    }

    void Scene::set_tileset(const std::string & uri) {
      tile_size = get_tileset_tile_size(uri);
      tilemap.tileset_uri = uri;
    }
    void Scene::set_state(const game::SceneState & state) {
      this->state = state;

      assert(state.cells.size() == _viewport.size);

      for(unsigned int j = 0 ; j < _viewport.size.y ; j ++) {
        for(unsigned int i = 0 ; i < _viewport.size.x ; i ++) {
          Vec2u pi(i, j);

          auto & tile = tilemap.tiles.get(pi);
          auto & cell = state.cells.get(pi);

          tile.tileset_index = cell.tile.glyph.index;
          tile.foreground_color.r = cell.tile.glyph.foreground.r;
          tile.foreground_color.g = cell.tile.glyph.foreground.g;
          tile.foreground_color.b = cell.tile.glyph.foreground.b;
          tile.background_color.r = cell.tile.glyph.background.r;
          tile.background_color.g = cell.tile.glyph.background.g;
          tile.background_color.b = cell.tile.glyph.background.b;
        }
      }
    }
    void Scene::add_missile() {
    }

    void Scene::tick() {
    }
    void Scene::draw() const {
      draw::clip(_draw_rect);
      Vec2i pixel_offset(
        (_draw_rect.size.x/2) - (_viewport.size.x*tile_size.x/2),
        (_draw_rect.size.y/2) - (_viewport.size.y*tile_size.y/2)
      );
      Vec2i pos = _draw_rect.pos + pixel_offset;
      tilemap_shader->draw(tilemap, pos);
      draw::unclip();
    }

    // rounds the given screen position to the nearest grid location
    Vec2i Scene::grid_pos(Vec2i screen_pos) const {
      return Vec2i(screen_pos.x / tile_size.x, screen_pos.y / tile_size.y) + _viewport.pos;
    }
    // returns the screen position of the center of the given grid location
    Vec2i Scene::screen_pos(Vec2i grid_pos) const {
      Vec2i local_grid_pos = grid_pos - _viewport.pos;
      return Vec2i(local_grid_pos.x * tile_size.x + tile_size.x / 2,
                   local_grid_pos.y * tile_size.y + tile_size.y / 2);
    }
  }
}

