#ifndef GAME_CORE_HPP
#define GAME_CORE_HPP

#include <vector>
#include <cassert>

namespace game {
  typedef unsigned long Id;

  class BaseEvent {
    const unsigned int _type_id;

    public:
    BaseEvent(unsigned int _type_id) : _type_id(_type_id) {}
    virtual ~BaseEvent() = default;
    unsigned int type_id() { return _type_id; }
  };

  class Object;

  class BasePart {
    const unsigned int _type_id;

    public:
    BasePart(unsigned int _type_id) : _type_id(_type_id) {}
    virtual ~BasePart() = default;
    unsigned int type_id() { return _type_id; }

    virtual void fire_event(BaseEvent & event) {}
  };

  class BasePartState {
    const unsigned int _type_id;

    public:
    BasePartState(unsigned int _type_id) : _type_id(_type_id) {}
    virtual ~BasePartState() = default;
    unsigned int type_id() { return _type_id; }
  };

  class BasePartFactory {
    public:
    virtual BasePart * create(unsigned int type_id) = 0;
    virtual void destroy(BasePart * part) = 0;
  };

  class Object {
    public:
    class Builder {
      std::vector<unsigned int> _type_ids;
      BasePartFactory & factory;

      public:
      Builder(BasePartFactory & factory)
        : factory(factory) {}

      Builder & add_part(unsigned int type_id) {
        _type_ids.push_back(type_id);
        return *this;
      }

      Builder & add_part(unsigned int type_id, const BasePartState & state);

      Object build() const {
        Object o(factory);
        for(auto & type_id : _type_ids) {
          o.parts.push_back(factory.create(type_id));
        }
        return o;
      }
    };

    ~Object() {
      for(auto & p : parts) {
        factory.destroy(p);
      }
    }

    Object(const Object & other) = delete;
    Object(Object && other)
      : factory(other.factory) {
      parts = other.parts;
      other.parts.clear();
    }
    Object & operator=(const Object & other) = delete;
    Object & operator=(Object && other) = delete;

    void fire_event(BaseEvent & event) {
      for(auto & p : parts) {
        if(p) {
          p->fire_event(event);
        }
      }
    }

    BasePart * get_part(unsigned int type_id) {
      for(auto & p : parts) {
        if(p) {
          if(p->type_id() == type_id) {
            return p;
          }
        }
      }
      return nullptr;
    }

    private:
    Object(BasePartFactory & factory)
      : factory(factory) {}

    std::vector<BasePart *> parts;

    BasePartFactory & factory;

    friend class builder;
  };
}

#endif
