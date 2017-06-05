#ifndef GAME_WORLD_HPP
#define GAME_WORLD_HPP

#include <string>
#include <set>
#include <game/core.hpp>
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

  class Entity {
    public:
    std::string name;

    world::Id location;
    Vec2i position;

    std::vector<Item> inventory;
    std::map<std::string, Item> equipment;

    nc::Entity ext;
  };

  class World {
    public:
    world::Id new_entity() {
      return ++ next_eid;
    }
    world::Id new_space() {
      return ++ next_sid;
    }

    std::map<world::Id, Entity> entities;
    std::map<world::Id, Level> levels;
    //std::map<world::Id, Player> players;

    private:
    world::Id next_eid = 0;
    world::Id next_sid = 0;
  };
}

#endif
