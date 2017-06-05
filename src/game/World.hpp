#ifndef GAME_WORLD_HPP
#define GAME_WORLD_HPP

#include <string>
#include <set>
#include <game/core.hpp>
#include <util/Vec2.hpp>
#include <util/Map.hpp>

namespace game {
  namespace eng {
  };

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

  typedef uint64_t WorldId;

  struct Space {
    Map<unsigned int> tiles;
    Map<unsigned int> opaque;

    std::vector<WorldItem> items;
    std::set<WorldId> entities;

    bool is_passable(Vec2i pos);
  };

  class Entity {
    public:
    std::string name;

    WorldId location;
    Vec2i position;

    std::vector<Item> inventory;
    std::map<std::string, Item> equipment;

    bool has_ai = false;
    bool has_turn = false;

    nc::Entity ext;
  };

  class World {
    public:
    WorldId new_entity() {
      return ++ next_eid;
    }
    WorldId new_space() {
      return ++ next_sid;
    }

    std::map<WorldId, Entity> entities;
    std::map<WorldId, Space> spaces;
    std::map<WorldId, WorldId> player_locations;

    private:
    WorldId next_eid = 0;
    WorldId next_sid = 0;
  };
}

#endif
