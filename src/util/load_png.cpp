
#include <png.h>

#include <util/Log.hpp>
#include <util/Image.hpp>

static Log fileLog;

bool load_png(Image & image, const char * filename)
{
  FILE * fp = fopen(filename, "rb");

  bool success = false;

  if(fp) {
    unsigned char header[8];

    fread(header, 1, 8, fp);

    if(!png_sig_cmp(header, 0, 8)) {
      // Header is good so far, get ready to read
      png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      png_infop info_ptr = NULL;

      if(png_ptr) {
        info_ptr = png_create_info_struct(png_ptr);

        if(info_ptr) {
          png_byte * data = NULL;

          if(!setjmp(png_jmpbuf(png_ptr))) {
            png_init_io(png_ptr, fp);
            png_set_sig_bytes(png_ptr, 8);

            png_read_info(png_ptr, info_ptr);

            int width = png_get_image_width(png_ptr, info_ptr);
            int height = png_get_image_height(png_ptr, info_ptr);
            int color_type = png_get_color_type(png_ptr, info_ptr);

            if((color_type == PNG_COLOR_TYPE_RGB || 
                 color_type == PNG_COLOR_TYPE_RGB_ALPHA ||
                 color_type == PNG_COLOR_TYPE_GRAY ||
                 color_type == PNG_COLOR_TYPE_GRAY_ALPHA)) {
              png_read_update_info(png_ptr, info_ptr);
              int row_bytes = png_get_rowbytes(png_ptr, info_ptr);

              data = new png_byte[row_bytes * height];

              //cout << "row_bytes: " << row_bytes << endl;
              for(int i = 0;i < height;i ++)
              {
                png_read_row(png_ptr, data + i*row_bytes, NULL);
              }

              fileLog.logf<Log::DEBUG0>("Read PNG %s: width=%u, height=%u, color_type=%u", filename, width, height, color_type);

              image.set_size(width, height);
              uint8_t * p = image.pixels();

              // Depending on the format, load the data into the Image struct
              if(color_type == PNG_COLOR_TYPE_RGB)
              {
                for(int i = 0;i < width*height;i ++)
                {
                  p[i*4 + 0] = data[i*3];
                  p[i*4 + 1] = data[i*3 + 1];
                  p[i*4 + 2] = data[i*3 + 2];
                  p[i*4 + 3] = 255;
                }
              }
              else if(color_type == PNG_COLOR_TYPE_RGB_ALPHA)
              {
                for(int i = 0;i < width*height;i ++)
                {
                  p[i*4 + 0] = data[i*4];
                  p[i*4 + 1] = data[i*4 + 1];
                  p[i*4 + 2] = data[i*4 + 2];
                  p[i*4 + 3] = data[i*4 + 3];
                }
              }
              else if(color_type == PNG_COLOR_TYPE_GRAY)
              {
                for(int i = 0;i < width*height;i ++)
                {
                  p[i*4 + 0] = data[i];
                  p[i*4 + 1] = data[i];
                  p[i*4 + 2] = data[i];
                  p[i*4 + 3] = 255;
                }
              }
              else if(color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
              {
                for(int i = 0;i < width*height;i ++)
                {
                  p[i*4 + 0] = data[i*2];
                  p[i*4 + 1] = data[i*2];
                  p[i*4 + 2] = data[i*2];
                  p[i*4 + 3] = data[i*2 + 1];
                }
              }

              success = true;
            } else {
              fileLog.logf<Log::WARNING>("PNG ERROR: Unsupported color type: %d.", color_type);
            }
          } else {
            fileLog.logf<Log::WARNING>("PNG ERROR: Failed to read png internals. (longjmp).");
          }

          delete [] data;
        } else {
          fileLog.logf<Log::WARNING>("PNG ERROR: png_create_info_struct() failed.");
        }

        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      } else {
        fileLog.logf<Log::WARNING>("PNG ERROR: png_create_read_struct() failed.");
      }
    } else {
      fileLog.logf<Log::WARNING>("PNG ERROR: %s is not a PNG file.", filename);
    }

    fclose(fp);
    fp = NULL;
  } else {
    fileLog.logf<Log::WARNING>("PNG ERROR: Could not open PNG file for reading at %s", filename);
  }

  return success;
}


