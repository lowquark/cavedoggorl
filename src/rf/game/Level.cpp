
#include "Level.hpp"

namespace rf {
  namespace game {
    Id Level::new_object_id() {
      return ++last_id;
    }
  }
}

