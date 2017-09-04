#ifndef RF_UTIL_LOG_HPP
#define RF_UTIL_LOG_HPP

#include <cstdio>
#include <cstdarg>

namespace rf {
  class Log {
    public:
    Log() : file(stdout) {}
    Log(FILE * f) : file(f) {}

    void logtopicvf(const char * topic, const char * fmt, va_list args) {
      fprintf(file, "XX:XX:XX [%s] ", topic);
      vfprintf(file, fmt, args);
      fputs("\n", file);
    }
    void logvf(const char * fmt, va_list args) {
      fprintf(file, "XX:XX:XX ");
      vfprintf(file, fmt, args);
      fputs("\n", file);
    }

    void logtopicf(const char * topic, const char * fmt, ...) {
      va_list args;
      va_start(args, fmt);
      logtopicvf(topic, fmt, args);
      va_end(args);
    }
    void logf(const char * fmt, ...) {
      va_list args;
      va_start(args, fmt);
      logvf(fmt, args);
      va_end(args);
    }

    void logtopic(const char * topic, const char * str) {
      fprintf(file, "XX:XX:XX [%s] %s\n", topic, str);
    }
    void log(const char * str) {
      fprintf(file, "XX:XX:XX %s\n", str);
    }

    void warntopicvf(const char * topic, const char * fmt, va_list args) {
      fprintf(file, "\x1b[33mXX:XX:XX [%s] \x1b[33;1mWARNING:\x1b[0;33m ", topic);
      vfprintf(file, fmt, args);
      fputs("\x1b[0m\n", file);
    }
    void warnvf(const char * fmt, va_list args) {
      fprintf(file, "\x1b[33mXX:XX:XX \x1b[33;1mWARNING:\x1b[0;33m ");
      vfprintf(file, fmt, args);
      fputs("\x1b[0m\n", file);
    }

    void warntopicf(const char * topic, const char * fmt, ...) {
      va_list args;
      va_start(args, fmt);
      warntopicvf(topic, fmt, args);
      va_end(args);
    }
    void warnf(const char * fmt, ...) {
      va_list args;
      va_start(args, fmt);
      warnvf(fmt, args);
      va_end(args);
    }
    void warntopic(const char * topic, const char * str) {
      fprintf(file, "\x1b[33mXX:XX:XX [%s] \x1b[33;1mWARNING:\x1b[0;33m %s\x1b[0m\n", topic, str);
    }
    void warn(const char * str) {
      fprintf(file, "\x1b[33mXX:XX:XX \x1b[33;1mWARNING:\x1b[0;33m %s\x1b[0m\n", str);
    }

    void flush() {
      fflush(file);
    }

    private:
    FILE * file;
  };

  void logtopicf(const char * topic, const char * fmt, ...);
  void logf(const char * fmt, ...);
  void logtopic(const char * topic, const char * str);
  void log(const char * str);

  void warntopicf(const char * topic, const char * fmt, ...);
  void warnf(const char * fmt, ...);
  void warntopic(const char * topic, const char * str);
  void warn(const char * str);
}

/*
  Log log;
  log.warn("hi");
  log.log("hi");
  log.warntopic("topic!", "hi");
  log.logtopic("topic!", "hi");
  log.warnf("hi %d", 5);
  log.logf("hi %d", 5);
  log.warntopicf("topic!", "hi %d", 5);
  log.logtopicf("topic!", "hi %d", 5);
*/

#endif
