
#include "Log.hpp"

const char * Log::levelStr[7] = { "  ERROR: ", "WARNING: ", "", "% ", "%  ", "%   ", "%    " };

void Log::setFile(FILE * f) {
  file = f;
}
FILE * Log::getFile() {
  return file;
}

void Log::flush() {
  fflush(file);
}

/*
static int evalFun() {
  printf("FUN!\n");
  return 5;
}

void testLog() {
  gg::logf<gg::DEBUG3>("%u [via gg::logf<gg::DEBUG3>(...)]", evalFun());
  gg::log<gg::DEBUG3>("%no %escape %sequences [via gg::log<gg::DEBUG3>(...)]");
  gg::logf<gg::DEBUG2>("%u [via gg::logf<gg::DEBUG2>(...)]", evalFun());
  gg::log<gg::DEBUG2>("%no %escape %sequences [via gg::log<gg::DEBUG2>(...)]");
  gg::logf<gg::DEBUG1>("%u [via gg::logf<gg::DEBUG1>(...)]", evalFun());
  gg::log<gg::DEBUG1>("%no %escape %sequences [via gg::log<gg::DEBUG1>(...)]");
  gg::logf<gg::DEBUG0>("%u [via gg::logf<gg::DEBUG0>(...)]", evalFun());
  gg::log<gg::DEBUG0>("%no %escape %sequences [via gg::log<gg::DEBUG0>(...)]");
  gg::logf<gg::INFO>("%u [via gg::logf<gg::INFO>(...)]", evalFun());
  gg::log<gg::INFO>("%no %escape %sequences [via gg::log<gg::INFO>(...)]");
  gg::logf<gg::WARNING>("%u [via gg::logf<gg::WARNING>(...)]", evalFun());
  gg::log<gg::WARNING>("%no %escape %sequences [via gg::log<gg::WARNING>(...)]");
  gg::logf<gg::ERROR>("%u [via gg::logf<gg::ERROR>(...)]", evalFun());
  gg::log<gg::ERROR>("%no %escape %sequences [via gg::log<gg::ERROR>(...)]");
  gg::logf<>("%u [via gg::logf<>(...)]", evalFun());
  gg::log<>("%no %escape %sequences [via gg::log<>(...)]");
  gg::logf("%u [via gg::logf(...)]", evalFun());
  gg::log("%no %escape %sequences [via gg::log(...)]");
  fflush(stdout);
}
*/

