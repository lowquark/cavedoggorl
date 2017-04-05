#ifndef LOG_HPP
#define LOG_HPP

/*
 * Log.hpp
 * log and logf
 *
 * Roughly inspired by http://www.drdobbs.com/cpp/logging-in-c/201804215
 *
 * Log messages from a program to a file. Unwanted log messages are not only
 * omitted, they are compiled to an empty function call. (easily optimized out)
 *
 * Additionally, evaluate all arguments and stay within their namespace, unlike the
 * following naieve macro implementation:
 *
 * constexpr LogLevel globalLogLevel = LOG_LEVEL;
 * #define LOG(level,fmt,...) \
 * if((level) <= globalLogLevel) \
 * log((level),(fmt),...)
 */

#ifndef LOG_LEVEL
// LOG_LEVEL may be defined as one of the following:
//
// ERROR, WARNING, INFO, DEBUG0, DEBUG1, DEBUG2, DEBUG3
#define LOG_LEVEL DEBUG3
#endif

#include <cstdio>
#include <cstdarg>
#include <type_traits>

// logf(fmt, ...) -- logs using fmt as a *printf format string for args (...)
// log(str) -- logs the string str

class Log {
  public:
  enum LogLevel { ERROR = 0, WARNING, INFO, DEBUG0, DEBUG1, DEBUG2, DEBUG3 };

  Log() : file(stdout) {}
  Log(FILE * f) : file(f) {}

  void setFile(FILE * f);
  FILE * getFile();

  template <LogLevel Tlevel = INFO>
  typename std::enable_if<(LOG_LEVEL >= Tlevel), void>::type logf(const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fputs(levelStr[Tlevel], file);
    vfprintf(file, fmt, args);
    fputs("\n", file);
    va_end(args);
  }
  template <LogLevel Tlevel = INFO>
  typename std::enable_if<(LOG_LEVEL >= Tlevel), void>::type log(const char * str) {
    fputs(levelStr[Tlevel], file);
    fputs(str, file);
    fputs("\n", file);
  }

  template <LogLevel Tlevel = INFO>
  typename std::enable_if<(LOG_LEVEL < Tlevel), void>::type logf(const char * fmt, ...) {
  }
  template <LogLevel Tlevel = INFO>
  typename std::enable_if<(LOG_LEVEL < Tlevel), void>::type log(const char * str) {
  }

#if LOG_LEVEL >= INFO
  inline void logf(const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fputs(levelStr[INFO], file);
    vfprintf(file, fmt, args);
    fputs("\n", file);
    va_end(args);
  }
  inline void log(const char * str) {
    fputs(levelStr[INFO], file);
    fputs(str, file);
    fputs("\n", file);
  }
#else
  inline void logf(const char * fmt, ...) {
  }
  inline void log(const char * str) {
  }
#endif

  void flush();

  private:
  FILE * file;
  static const char * levelStr[7];
};

#endif
