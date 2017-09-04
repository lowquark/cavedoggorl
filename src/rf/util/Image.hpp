#ifndef RF_IMAGE_HPP
#define RF_IMAGE_HPP

#include <cstdint>

namespace rf {
  // Always RGBA
  class Image
  {
    unsigned int _width = 0;
    unsigned int _height = 0;
    uint8_t * _pixels = nullptr;

    public:
    Image() noexcept = default;
    ~Image();
    Image(unsigned int w, unsigned int h);

    Image(const Image & other) = delete;
    Image & operator=(const Image & other) = delete;
    Image(Image && other) noexcept;
    Image & operator=(Image && other) noexcept;

    void set_size(unsigned int w, unsigned int h);

    uint8_t * pixels() const noexcept { return _pixels; }
    unsigned int width() const noexcept { return _width; }
    unsigned int height() const noexcept { return _height; }
  };
}

#endif
