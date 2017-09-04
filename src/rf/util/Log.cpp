
#include "Log.hpp"

namespace rf {
  Log global_log;

  void logtopicf(const char * topic, const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    global_log.logtopicvf(topic, fmt, args);
    va_end(args);
  }
  void logf(const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    global_log.logvf(fmt, args);
    va_end(args);
  }
  void logtopic(const char * topic, const char * str) {
    global_log.logtopic(topic, str);
  }
  void log(const char * str) {
    global_log.log(str);
  }

  void warntopicf(const char * topic, const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    global_log.warntopicvf(topic, fmt, args);
    va_end(args);
  }
  void warnf(const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    global_log.warnvf(fmt, args);
    va_end(args);
  }
  void warntopic(const char * topic, const char * str) {
    global_log.warntopic(topic, str);
  }
  void warn(const char * str) {
    global_log.warn(str);
  }
}

