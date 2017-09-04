
#include "Image.hpp"

#include <string.h>

namespace rf {
  Image::~Image() {
    delete [] _pixels;
  }
  Image::Image(unsigned int w, unsigned int h)
    : _width(w)
    , _height(h)
    , _pixels(nullptr) {
    _pixels = new uint8_t[w*h*4];
  }

  Image::Image(Image && other) noexcept {
    _width = other._width;
    _height = other._height;
    _pixels = other._pixels;
    other._width = 0;
    other._height = 0;
    other._pixels = nullptr;
  }
  Image & Image::operator=(Image && other) noexcept {
    if(this != &other) {
      delete [] _pixels;
      _width = other._width;
      _height = other._height;
      _pixels = other._pixels;
      other._width = 0;
      other._height = 0;
      other._pixels = nullptr;
    }
    return *this;
  }

  void Image::set_size(unsigned int w, unsigned int h) {
    delete [] _pixels;
    _width = w;
    _height = h;
    _pixels = new uint8_t[_width*_height*4];
    memset(_pixels, 0, _width*_height*4);
  }
}

