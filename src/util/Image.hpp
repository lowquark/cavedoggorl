#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <stdint.h>
#include <stddef.h>

// Always RGBA
class Image
{
  unsigned int _width = 0;
  unsigned int _height = 0;
  uint8_t * _pixels = nullptr;

  public:
  Image() = default;
  ~Image();
  Image(unsigned int w, unsigned int h);

  Image(const Image & other) = delete;
  Image & operator=(const Image & other) = delete;
  Image(Image && other);
  Image & operator=(Image && other);

  void set_size(unsigned int w, unsigned int h);

  uint8_t * pixels() const { return _pixels; }
  unsigned int width() const { return _width; }
  unsigned int height() const { return _height; }
};

#endif
