#ifndef RF_UTIL_LOG_HPP
#define RF_UTIL_LOG_HPP

#include <cstdio>
#include <cstdarg>
#include <ctime>

namespace rf {
  class Log {
    public:
    Log() : file(stdout) {}
    Log(FILE * f) : file(f) {}
    virtual ~Log() = default;

    void get_timestr(char (*str)[10]) {
      time_t t = time(NULL);
      struct tm time_val;
      localtime_r(&t, &time_val);
      strftime(*str, 10, "%H:%M:%S ", &time_val);
    }

    virtual void logvf(const char * fmt, va_list args) {
      char timestr[10];
      get_timestr(&timestr);
      fputs(timestr, file);
      vfprintf(file, fmt, args);
      fputs("\n", file);
    }
    virtual void logf(const char * fmt, ...) {
      va_list args;
      va_start(args, fmt);
      logvf(fmt, args);
      va_end(args);
    }
    virtual void log(const char * str) {
      char timestr[10];
      get_timestr(&timestr);
      fputs(timestr, file);
      fputs(str, file);
      fputs("\n", file);
    }

    virtual void warnvf(const char * fmt, va_list args) {
      char timestr[10];
      get_timestr(&timestr);
      fputs("\x1b[33m", file);
      fputs(timestr, file);
      fputs("\x1b[33;1mWARNING:\x1b[0;33m ", file);
      vfprintf(file, fmt, args);
      fputs("\x1b[0m\n", file);
    }
    virtual void warnf(const char * fmt, ...) {
      va_list args;
      va_start(args, fmt);
      warnvf(fmt, args);
      va_end(args);
    }
    virtual void warn(const char * str) {
      char timestr[10];
      get_timestr(&timestr);
      fputs("\x1b[33m", file);
      fputs(timestr, file);
      fputs("\x1b[33;1mWARNING:\x1b[0;33m ", file);
      fputs(str, file);
      fputs("\x1b[0m\n", file);
    }

    void flush() {
      fflush(file);
    }

    protected:
    FILE * file;
  };

  class LogTopic : public Log {
    public:
    LogTopic(const char * topic) : Log(stdout), topic(topic) {}
    LogTopic(const char * topic, FILE * f) : Log(f), topic(topic) {}

    virtual void logvf(const char * fmt, va_list args) {
      char timestr[10];
      get_timestr(&timestr);
      fputs(timestr, file);
      fputs("[", file);
      fputs(topic, file);
      fputs("] ", file);
      vfprintf(file, fmt, args);
      fputs("\n", file);
    }
    virtual void logf(const char * fmt, ...) {
      va_list args;
      va_start(args, fmt);
      logvf(fmt, args);
      va_end(args);
    }
    virtual void log(const char * str) {
      char timestr[10];
      get_timestr(&timestr);
      fputs(timestr, file);
      fputs("[", file);
      fputs(topic, file);
      fputs("] ", file);
      fputs(str, file);
      fputs("\n", file);
    }

    virtual void warnvf(const char * fmt, va_list args) {
      char timestr[10];
      get_timestr(&timestr);
      fputs("\x1b[33m", file);
      fputs(timestr, file);
      fputs("[", file);
      fputs(topic, file);
      fputs("] \x1b[33;1mWARNING:\x1b[0;33m ", file);
      vfprintf(file, fmt, args);
      fputs("\x1b[0m\n", file);
    }
    virtual void warnf(const char * fmt, ...) {
      va_list args;
      va_start(args, fmt);
      warnvf(fmt, args);
      va_end(args);
    }
    virtual void warn(const char * str) {
      char timestr[10];
      get_timestr(&timestr);
      fputs("\x1b[33m", file);
      fputs(timestr, file);
      fputs("[", file);
      fputs(topic, file);
      fputs("] \x1b[33;1mWARNING:\x1b[0;33m ", file);
      fputs(str, file);
      fputs("\x1b[0m\n", file);
    }

    private:
    const char * topic;
  };

  LogTopic & logtopic(const char * topic);

  void logf(const char * fmt, ...);
  void log(const char * str);

  void warnf(const char * fmt, ...);
  void warn(const char * str);
}

/*
rf::warn("hi");
rf::log("hi");
rf::warnf("hi %d", 5);
rf::logf("hi %d", 5);

rf::logtopic("topic!").log("hi");
rf::logtopic("topic!").warn("hi");
rf::logtopic("topic!").logf("hi %d", 5);
rf::logtopic("topic!").warnf("hi %d", 5);
*/

#endif
