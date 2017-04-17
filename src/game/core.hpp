#ifndef GAME_CORE_HPP
#define GAME_CORE_HPP

#include <vector>

namespace game {
  typedef unsigned long Id;

  class Event {
    const unsigned int _type_id;

    public:
    Event(unsigned int _type_id) : _type_id(_type_id) {}
    virtual ~Event() = default;
    unsigned int type_id() { return _type_id; }
  };

  class Property {
    const unsigned int _type_id;

    public:
    Property(unsigned int _type_id) : _type_id(_type_id) {}
    virtual ~Property() = default;
    unsigned int type_id() { return _type_id; }

    virtual void fire_event(Event & event) {}
  };

  /*
  //TODO
  class PropertyState {
  };
  */

  class BasePropertyFactory {
    public:
    virtual Property * create(unsigned int type_id) = 0;
    virtual void destroy(Property * property) = 0;
  };

  class Object {
    public:
    class Builder {
      std::vector<unsigned int> type_ids;
      BasePropertyFactory & factory;

      public:
      Builder(BasePropertyFactory & factory)
        : factory(factory) {}

      Builder & add_property(unsigned int type_id) {
        type_ids.push_back(type_id);
        return *this;
      }
      Object build() const {
        Object o(factory);
        for(auto & type_id : type_ids) {
          o.properties.push_back(factory.create(type_id));
        }
        return o;
      }
    };

    ~Object() {
      for(auto & p : properties) {
        factory.destroy(p);
      }
    }

    Object(const Object & other) = delete;
    Object(Object && other)
      : factory(other.factory) {
      properties = other.properties;
      other.properties.clear();
    }
    Object & operator=(const Object & other) = delete;
    Object & operator=(Object && other) = delete;

    void fire_event(Event & event);

    private:
    Object(BasePropertyFactory & factory)
      : factory(factory) {}

    std::vector<Property *> properties;

    BasePropertyFactory & factory;

    friend class builder;
  };
}

#endif
