
#include "GameSave.hpp"

namespace rf {
  namespace game {
    void GameSave::open() {
      close();
    }
    void GameSave::close() {
    }

    Id GameSave::player_level_id() const {
      return 1;
    }
    void GameSave::set_player_level_id(Id id) {
    }

    Id GameSave::player_object_id() const {
      return 1;
    }
    void GameSave::set_player_object_id(Id id) {
    }

    Level GameSave::level(Id id) {
      assert(id == 1);

      Vec2u level_size(30, 30);

      Level lv;
      lv.tiles.resize(level_size);

      srand(time(NULL));

      lv.objects[1].add(new BasicObjectGlyph(Glyph(3, Color(0xFF, 0xCC, 0x99))));
      lv.objects[2].add(new BasicObjectGlyph(Glyph(0, Color(0xFF, 0xCC, 0x99))));
      lv.objects[3].add(new BasicObjectGlyph(Glyph(1, Color(0xFF, 0xCC, 0x99))));
      lv.objects[4].add(new BasicObjectGlyph(Glyph(2, Color(0xFF, 0xCC, 0x99))));

      lv.objects[1].set_pos(Vec2i(5, 5));
      lv.objects[2].set_pos(Vec2i(rand() % level_size.x, rand() % level_size.y));
      lv.objects[3].set_pos(Vec2i(rand() % level_size.x, rand() % level_size.y));
      lv.objects[4].set_pos(Vec2i(rand() % level_size.x, rand() % level_size.y));

      lv.objects[1].set_has_turn(true);
      lv.objects[2].set_has_turn(true);
      lv.objects[3].set_has_turn(true);
      lv.objects[4].set_has_turn(true);

      lv.objects[1].set_playable(true);

      Color bg_color(0x11, 0x22, 0x22);

      for(unsigned int j = 0 ; j < level_size.y ; j ++) {
        for(unsigned int i = 0 ; i < level_size.x ; i ++) {
          Vec2u pi(i, j);
          auto & tile = lv.tiles.get(pi);
          tile.add(new BasicTileGlyph(
            Glyph(4, bg_color, bg_color)
          ));
        }
      }

      for(unsigned int j = 0 ; j < 10 ; j ++) {
        Vec2i drunkard((rand() % 16) + 8, (rand() % 16) + 8);
        for(unsigned int i = 0 ; i < 50 ; i ++) {
          drunkard.x += (rand() % 3) - 1;
          drunkard.y += (rand() % 3) - 1;

          if(drunkard.x < 0) { drunkard.x = 0; }
          if(drunkard.x > level_size.x - 1) { drunkard.x = level_size.x - 1; }
          if(drunkard.y < 0) { drunkard.y = 0; }
          if(drunkard.y > level_size.y - 1) { drunkard.y = level_size.y - 1; }

          auto & tile = lv.tiles[drunkard];
          tile = Tile();
          tile.add(new BasicTileGlyph(
            Glyph(
              5 + ((rand() % 2) + 6)*16,
              Color(0x88, 0x66, 0x44),
              bg_color
            )
          ));
        }
      }

      int min_height = 1;
      int height = 2;
      int max_height = 4;
      for(unsigned int x = 0 ; x < level_size.x ; x ++) {
        for(int y = 0 ; y < height ; y ++) {
          Vec2u pi(x, y);
          auto & tile = lv.tiles[pi];
          tile = Tile();
          tile.add(new BasicTileGlyph(
            Glyph(
              (rand() % 3) + 5 + 4*16,
              y >= height - 2 ? Color(0x33, 0x66, 0x33) : Color(0x22, 0x44, 0x44),
              bg_color
            )
          ));
        }
        height += (rand() % 3) - 1;
        if(height < min_height) { height = min_height; }
        if(height > max_height) { height = max_height; }
      }

      height = 1;
      //int max_height = 2;
      for(unsigned int x = 0 ; x < level_size.x ; x ++) {
        for(int y = level_size.y - height ; y < level_size.y ; y ++) {
          Vec2u pi(x, y);
          auto & tile = lv.tiles[pi];
          tile = Tile();
          tile.add(new BasicTileGlyph(
            Glyph(
              (rand() % 3) + 8 + 3*16,
              Color(0x88, 0x66, 0x44),
              bg_color
            )
          ));
        }
        height += (rand() % 3) - 1;
        if(height < min_height) { height = min_height; }
        if(height > max_height) { height = max_height; }
      }

      return lv;
    }
    void GameSave::set_level(Id id, const Level & l) {
    }
  }
}

