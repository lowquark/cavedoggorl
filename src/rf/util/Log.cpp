
#include "Log.hpp"

#include <string>
#include <map>

namespace rf {
  Log global_log;

  std::map<std::string, LogTopic> global_log_topics;

  LogTopic & logtopic(const char * topic) {
    std::string key(topic);
    auto kvpair_it = global_log_topics.find(key);
    if(kvpair_it == global_log_topics.end()) {
      auto kvpair = global_log_topics.emplace(std::make_pair(key, LogTopic(topic)));
      return kvpair.first->second;
    } else {
      return kvpair_it->second;
    }
  }

  void logf(const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    global_log.logvf(fmt, args);
    va_end(args);
  }
  void log(const char * str) {
    global_log.log(str);
  }

  void warnf(const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    global_log.warnvf(fmt, args);
    va_end(args);
  }
  void warn(const char * str) {
    global_log.warn(str);
  }
}

