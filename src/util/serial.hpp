#ifndef UTIL_SERIAL_HPP
#define UTIL_SERIAL_HPP

#include <iostream>
#include <cstdint>

namespace serial {
  void write(std::ostream & os, uint8_t v);
  bool read(std::istream & is, uint8_t & v);
  void write(std::ostream & os, int8_t v);
  bool read(std::istream & is, int8_t & v);

  void write(std::ostream & os, uint16_t v);
  bool read(std::istream & is, uint16_t & v);
  void write(std::ostream & os, int16_t v);
  bool read(std::istream & is, int16_t & v);
}

#endif
