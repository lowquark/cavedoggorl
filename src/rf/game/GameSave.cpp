
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

      Vec2u level_size(32, 32);

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

      for(unsigned int j = 0 ; j < level_size.y ; j ++) {
        for(unsigned int i = 0 ; i < level_size.x ; i ++) {
          Vec2u pi(i, j);
          auto & tile = lv.tiles.get(pi);
          unsigned int r = (rand() % 100);
          if(r <= 8) {
            tile.add(new BasicTileGlyph(Glyph(5 + 7*16, Color(0x33, 0x66, 0x33))));
          } else {
            tile.add(new BasicTileGlyph(Glyph(5 + 6*16, Color(0x22, 0x44, 0x44))));
          }
        }
      }

      return lv;
    }
    void GameSave::set_level(Id id, const Level & l) {
    }
  }
}

