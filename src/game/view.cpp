
#include "view.hpp"

namespace game {
  // Its big, it's heavy, it's wood
  Log log;
  // Better than bad, it's good!

  static View * _view = nullptr;
  void set_view(View & view) {
    _view = &view;
  }
  // Always valid!
  View & view() {
    static View default_view;
    if(_view) {
      return *_view;
    }
    return default_view;
  }
}

