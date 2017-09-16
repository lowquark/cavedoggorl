
#include "Level.hpp"

namespace rf {
  namespace game {
    Id Level::new_object_id() const {
      if(objects.empty()) {
        return 1;
      } else {
        return objects.end()->first + 1;
      }
    }
  }
}

