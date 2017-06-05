#ifndef WORLD_WORLD_HPP
#define WORLD_WORLD_HPP

#include <random>
#include <string>
#include <util/Map.hpp>

// world: persistent, stored state
namespace world {
  typedef uint64_t Id;
  typedef uint32_t TypeId;

  struct Tile {
    TypeId type_id = 0;

    Tile() = default;
    Tile(TypeId type_id) : type_id(type_id) {}
  };

  struct Level {
    Map<Tile> tiles;
    std::vector<Id> entities;
  };

  struct Entity {
    std::string name;

    Id location;
    Vec2i position;

    TypeId type_id;
    //std::string ext_state;
  };

  struct Player {
    Id entity = 0;
  };

  class WorldStore {
    public:
    class NotFoundException : public std::exception {
      virtual const char * what() const throw() {
        return "failed to find world object";
      }
    };
    class OutOfIdsException : public std::exception {
      virtual const char * what() const throw() {
        return "the world is out of ids";
      }
    };

    virtual Level level(Id id) const = 0;
    virtual Id    new_level(const Level & level) = 0;
    virtual void  save_level(Id id, const Level & level) = 0;
    virtual void  delete_level(Id id) = 0;

    virtual Entity entity(Id id) const = 0;
    virtual Id     new_entity(const Entity & entity) = 0;
    virtual void   save_entity(Id id, const Entity & entity) = 0;
    virtual void   delete_entity(Id id) = 0;

    virtual Player player(Id id) const = 0;
    virtual Id     new_player(const Player & player) = 0;
    virtual void   save_player(Id id, const Player & player) = 0;
    virtual void   delete_player(Id id) = 0;
  };
  class SPCaveWorld : public WorldStore {
    public:
    Level level(Id id) const override {
      //std::random_device rd;
      std::mt19937 gen(id);

      Level l;

      Vec2u size(60, 60);
      l.tiles.resize(size);
      for(unsigned int j = 0 ; j < size.y ; j ++) {
        for(unsigned int i = 0 ; i < size.x ; i ++) {
          Vec2i pos(i, j);
          if(gen() % 5 == 0) {
            l.tiles.set(pos, 1);
          } else {
            l.tiles.set(pos, 2);
          }
        }
      }

      if(id == 0) {
        l.entities.push_back(0);
        l.entities.push_back(1);
      }

      return l;
    }
    Id new_level(const Level & level) override {
      throw OutOfIdsException();
    }
    void save_level(Id id, const Level & level) override {
    }
    void delete_level(Id id) override {
    }

    Entity entity(Id id) const override {
      if(id == 0) {
        Entity the_player_entity;
        the_player_entity.name = "lambdoggo";

        the_player_entity.location = 0;
        the_player_entity.position = Vec2i(2, 2);

        the_player_entity.type_id = 0;
        return the_player_entity;
      } else if(id == 1) {
        Entity the_monster_entity;
        the_monster_entity.name = "ogreator";

        the_monster_entity.location = 0;
        the_monster_entity.position = Vec2i(20, 20);

        the_monster_entity.type_id = 0;
        return the_monster_entity;
      } else {
        throw NotFoundException();
      }
    }
    Id new_entity(const Entity & entity) override {
      throw OutOfIdsException();
    }
    void save_entity(Id id, const Entity & entity) override {
    }
    void delete_entity(Id id) override {
    }

    Player player(Id id) const override {
      if(id != 0) {
        throw NotFoundException();
      }

      Player the_player;
      the_player.entity = 0;
      return the_player;
    }
    Id new_player(const Player & player) override {
      throw OutOfIdsException();
    }
    void save_player(Id id, const Player & player) override {
    }
    void delete_player(Id id) override {
    }
  };
}

#endif
