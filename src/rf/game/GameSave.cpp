
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

    Tile grass_tile() {
      Color color(0x11, 0x22, 0x22);
      Tile tile;
      tile.add(new BasicTileGlyph(
        Glyph(4 + 1*16, color, color)
      ));
      return tile;
    }
    Tile grass_dirt_tile() {
      Color fg_color(0x88, 0x66, 0x44);
      Color bg_color(0x11, 0x22, 0x22);
      Tile tile;
      tile.add(new BasicTileGlyph(
        Glyph(5 + 7*16, fg_color, bg_color)
      ));
      return tile;
    }
    Tile grass_path_tile() {
      Color fg_color(0x88, 0x66, 0x44);
      Color bg_color(0x11, 0x22, 0x22);
      Tile tile;
      tile.add(new BasicTileGlyph(
        Glyph(5 + 8*16, fg_color, bg_color)
      ));
      return tile;
    }
    Tile grass_pine_needles() {
      Color fg_color(0x22, 0x44, 0x44);
      Color bg_color(0x11, 0x22, 0x22);
      Tile tile;
      tile.add(new BasicTileGlyph(
        Glyph(5 + 7*16, fg_color, bg_color)
      ));
      return tile;
    }

    Object tree() {
      Object obj;
      obj.add(new BasicObjectGlyph(
        Glyph(
          (rand() % 3) + 5 + 5*16,
          Color(0x33, 0x66, 0x33)
        )
      ));
      return obj;
    }
    Object rock() {
      Object obj;
      obj.add(new BasicObjectGlyph(
        Glyph(
          (rand() % 3) + 8 + 4*16,
          Color(0x88, 0x66, 0x44)
        )
      ));
      return obj;
    }

    Level GameSave::level(Id id) {
      assert(id == 1);

      Vec2u level_size(30, 30);

      Level lv;
      lv.tiles.resize(level_size);

      auto & doggo = lv.objects[lv.new_object_id()];
      doggo.add(new BasicObjectGlyph(Glyph(3 + 1*16, Color(0xFF, 0xCC, 0x99))));
      doggo.set_pos(Vec2i(5, 5));
      doggo.set_has_turn(true);
      doggo.set_playable(true);

      srand(time(NULL));

      Map<int> tile_ids(level_size);

      for(unsigned int j = 0 ; j < level_size.y ; j ++) {
        for(unsigned int i = 0 ; i < level_size.x ; i ++) {
          tile_ids[Vec2u(i, j)] = 0; // grass_tile
        }
      }

      // drunken walk trees
      for(unsigned int j = 0 ; j < 10 ; j ++) {
        Vec2i drunkard((rand() % level_size.x), (rand() % level_size.y));
        for(unsigned int i = 0 ; i < 50 ; i ++) {
          drunkard.x += (rand() % 3) - 1;
          drunkard.y += (rand() % 3) - 1;

          if(drunkard.x < 0) { drunkard.x = 0; }
          if(drunkard.x > level_size.x - 1) { drunkard.x = level_size.x - 1; }
          if(drunkard.y < 0) { drunkard.y = 0; }
          if(drunkard.y > level_size.y - 1) { drunkard.y = level_size.y - 1; }

          tile_ids[drunkard] = 1; // here be trees
        }
      }

      // add randomly placed trees
      for(int i = 0 ; i < 20 ; i ++) {
        tile_ids[Vec2u(rand() % level_size.x, rand() % level_size.y)] = 1; // here be trees
      }

      // add trees on top
      int min_height = 1;
      int height = 2;
      int max_height = 4;
      for(unsigned int x = 0 ; x < level_size.x ; x ++) {
        for(int y = 0 ; y < height ; y ++) {
          tile_ids[Vec2u(x, y)] = 1; // here be trees
        }
        height += (rand() % 3) - 1;
        if(height < min_height) { height = min_height; }
        if(height > max_height) { height = max_height; }
      }

      // add rocks on bottom

      height = 1;
      //int max_height = 2;
      for(unsigned int x = 0 ; x < level_size.x ; x ++) {
        for(int y = level_size.y - height ; y < level_size.y ; y ++) {
          tile_ids[Vec2u(x, y)] = 2; // here be rocks
        }
        height += (rand() % 3) - 1;
        if(height < min_height) { height = min_height; }
        if(height > max_height) { height = max_height; }
      }

      // populate region based on generation
      for(unsigned int j = 0 ; j < level_size.y ; j ++) {
        for(unsigned int i = 0 ; i < level_size.x ; i ++) {
          int id = tile_ids[Vec2u(i, j)];
          if(id == 0) {
            int r = rand() % 10;
            if(r == 0) {
              lv.tiles[Vec2u(i, j)] = grass_dirt_tile();
            } else if(r == 1) {
              lv.tiles[Vec2u(i, j)] = grass_path_tile();
            } else {
              lv.tiles[Vec2u(i, j)] = grass_tile();
            }
          } else if(id == 1) {
            auto & tree_obj = lv.objects[lv.new_object_id()];
            tree_obj = tree();
            tree_obj.set_pos(Vec2i(i, j));
            lv.tiles[Vec2u(i, j)] = grass_pine_needles();
          } else {
            auto & rock_obj = lv.objects[lv.new_object_id()];
            rock_obj = rock();
            rock_obj.set_pos(Vec2i(i, j));
            lv.tiles[Vec2u(i, j)] = grass_path_tile();
          }
        }
      }

      auto & orc = lv.objects[lv.new_object_id()];
      orc.add(new BasicObjectGlyph(Glyph(0 + 1*16, Color(0xFF, 0xCC, 0x99))));
      orc.set_pos(Vec2i(rand() % level_size.x, rand() % level_size.y));
      orc.set_has_turn(true);

      auto & nymph = lv.objects[lv.new_object_id()];
      nymph.add(new BasicObjectGlyph(Glyph(1 + 1*16, Color(0xFF, 0xCC, 0x99))));
      nymph.set_pos(Vec2i(rand() % level_size.x, rand() % level_size.y));
      nymph.set_has_turn(true);

      auto & wizard = lv.objects[lv.new_object_id()];
      wizard.add(new BasicObjectGlyph(Glyph(2 + 1*16, Color(0xFF, 0xCC, 0x99))));
      wizard.set_pos(Vec2i(rand() % level_size.x, rand() % level_size.y));
      wizard.set_has_turn(true);

      return lv;
    }
    void GameSave::set_level(Id id, const Level & l) {
    }
  }
}

