
#include "serial.hpp"

namespace serial {
  void write(std::ostream & os, uint8_t v) {
    os.write((const char *)&v, 1);
  }
  bool read(std::istream & is, uint8_t & v) {
    is.read((char *)&v, 1);
    return !is.fail();
  }
  void write(std::ostream & os, int8_t v) {
    write(os, (uint8_t)v);
  }
  bool read(std::istream & is, int8_t & v) {
    return read(is, (uint8_t &)v);
  }

  void write(std::ostream & os, uint16_t v) {
    uint8_t data[2];
    data[0] = (v >> 8) & 0xFF;
    data[1] = (v     ) & 0xFF;
    os.write((const char *)data, 2);
  }
  bool read(std::istream & is, uint16_t & v) {
    uint8_t data[2];
    is.read((char *)&data, 2);
    if(is.fail()) { return false; }
    v = (data[0] << 8) | data[1];
    return true;
  }
  void write(std::ostream & os, int16_t v) {
    write(os, (uint16_t)v);
  }
  bool read(std::istream & is, int16_t & v) {
    return read(is, (uint16_t &)v);
  }
}

