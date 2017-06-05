#ifndef GAME_WORLD_HPP
#define GAME_WORLD_HPP

#include <string>
#include <set>
#include <game/core.hpp>
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

  struct Space {
    Map<unsigned int> tiles;
    Map<unsigned int> opaque;

    std::vector<WorldItem> items;
    std::set<nc::Id> entities;

    bool is_passable(Vec2i pos);
  };

  class Entity {
    public:
    std::string name;

    std::string location;
    Vec2i position;

    std::vector<Item> inventory;
    std::map<std::string, Item> equipment;

    bool has_ai = false;
    bool has_turn = false;

    nc::Entity ext;
  };

  // A loaded representation of the world, definitely not the entire thing
  struct World {
    nc::IdSet<Entity> entities;
    std::map<std::string, Space> spaces;
  };
}

#endif
