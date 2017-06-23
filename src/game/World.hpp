#ifndef GAME_WORLD_HPP
#define GAME_WORLD_HPP

#include <string>
#include <set>
#include <game/core.hpp>
#include <game/actions.hpp>
#include <world/world.hpp>
#include <util/Vec2.hpp>
#include <util/Map.hpp>

namespace game {
  class ItemType {
    public:
    std::string name;

    nc::Entity ext;

    ItemType(unsigned int id) : _id(id) {}

    unsigned int id() const { return _id; }

    private:
    unsigned int _id;
  };
  class Item {
    public:
    unsigned int quantity = 0;

    Item() = default;
    Item(const ItemType * type) : _type(type) {}

    const ItemType * type() const { return _type; }

    private:
    const ItemType * _type = nullptr;
  };

  struct WorldItem {
    Vec2i position;
    Item item;
  };

  struct Level {
    Map<unsigned int> tiles;
    Map<unsigned int> opaque;

    std::vector<WorldItem> items;
    std::set<world::Id> entities;

    bool is_passable(Vec2i pos);
  };

  class Player;

  class Entity {
    public:
    std::string name;

    world::Id location;
    Vec2i position;

    Player * player = nullptr;

    std::vector<Item> inventory;
    std::map<std::string, Item> equipment;

    nc::Entity ext;
  };

  class View {
    public:
    virtual ~View() = default;

    struct TileState {
      unsigned int glyph_id = 0;
      bool visible = true;
    };
    struct EntityState {
      Vec2i pos;
      unsigned int glyph_id = 0;
    };


    const std::string & location_name() const { return _location_name; }
    const Map<TileState> & tiles() const { return _tiles; }
    const std::map<unsigned int, EntityState> & entities() const { return _entities; }
    const std::pair<bool, unsigned int> & player_entity_id() const { return _player_entity_id; }

    void set_location_name(const std::string & str) {
      _location_name = str;
      notify_location_name_update();
    }
    void set_tiles(const Map<TileState> & tiles) {
      _tiles = tiles;
      notify_tiles_update();
    }

    void clear_entities() {
      _entities.clear();
      notify_clear_entities();
    }
    void set_entity(unsigned int id, const EntityState & state) {
      _entities[id] = state;
      notify_entity_update(id);
    }
    void clear_entity(unsigned int id) {
      _entities.erase(id);
      notify_entity_remove(id);
    }

    void move_entity(unsigned int id, Vec2i from, Vec2i to) {
      auto kvpair_it = _entities.find(id);
      if(kvpair_it != _entities.end()) {
        auto & e = kvpair_it->second;
        e.pos = to;
        notify_entity_move(id, from, to);
      }
    }

    private:
    std::string _location_name;
    Map<TileState> _tiles;
    std::map<unsigned int, EntityState> _entities;
    std::pair<bool, unsigned int> _player_entity_id = decltype(_player_entity_id)(false, 0);


    virtual void notify_update_tile(Vec2i pos) {}
    virtual void notify_tiles_update() {}

    virtual void notify_clear_entities() {}
    virtual void notify_entity_update(unsigned int id) {};
    virtual void notify_entity_remove(unsigned int id) {}
    virtual void notify_entity_show(unsigned int id) {}
    virtual void notify_entity_hide(unsigned int id) {}
    // Could probably be a little more creative with deducing from/to
    virtual void notify_entity_move(unsigned int id, Vec2i from, Vec2i to) {}
    virtual void notify_player_update() {}
    virtual void notify_fov_update() {}

    virtual void notify_location_name_update() {}
    virtual void notify_message(const std::string & message) {}
  };

  struct PlayerLevelData {
    Map<unsigned int> seen;
  };

  class Player {
    public:
    world::Id entity = 0;
    std::unique_ptr<Action> next_action;
    std::map<world::Id, PlayerLevelData> level_data;
  };

  class World {
    public:
    std::map<world::Id, Entity> entities;
    std::map<world::Id, Level> levels;
    std::map<world::Id, Player> players;
  };
}

#endif
