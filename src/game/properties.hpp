#ifndef GAME_PROPERTIES_HPP
#define GAME_PROPERTIES_HPP

#include <game/core.hpp>

namespace game {
  static const unsigned int PROPERTY_TYPE_OBSTRUCTION = 1;

  class ObstructionProperty : public Property {
    public:
    ObstructionProperty() : Property(PROPERTY_TYPE_OBSTRUCTION) {}
  };

  class PropertyFactory : public BasePropertyFactory {
    public:
    Property * create(unsigned int type_id) override {
      switch(type_id) {
        case PROPERTY_TYPE_OBSTRUCTION:
          return new ObstructionProperty;
        default:
          return nullptr;
      }
    }
    void destroy(Property * property) override {
      delete property;
    }
  };

  extern PropertyFactory property_factory; 
}

#endif
