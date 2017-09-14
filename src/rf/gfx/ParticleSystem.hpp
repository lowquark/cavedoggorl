#ifndef RF_GFX_PARTICLESYSTEM_HPP
#define RF_GFX_PARTICLESYSTEM_HPP

#include <cassert>
#include <memory>
#include <rf/gfx/Tilemap.hpp>
#include <rf/gfx/Color.hpp>

namespace rf {
  namespace gfx {
    /*
    class ParticleSystem {
      public:
      ParticleSystem() {
      }
      ParticleSystem(const ParticleSystem & other) = delete;
      ParticleSystem & operator=(const ParticleSystem & other) = delete;
      ~ParticleSystem() {
        free(particles);
        particles = nullptr;
      }

      static void test() {
        ParticleSystem sys;
        sys.resize(50);
        for(int i = 0 ; i < 100 ; i ++ ) {
          if(i < 50) {
            Particle * p = sys.alloc_particle();
            assert(p);
          } else {
            Particle * p = sys.alloc_particle();
            assert(p == nullptr);
          }
        }
      }

      void resize(unsigned int max) {
        assert(sizeof(Particle) >= sizeof(Particle *));

        if(particles) {
          free(particles);
          particles = nullptr;
        }
        particles = (Particle *)malloc(sizeof(Particle)*max);

        next_free = particles;
        for(unsigned int i = 0 ; i < max - 1 ; i ++) {
          *((Particle **)&particles[i]) = &particles[i + 1];
        }
        *((Particle **)&particles[max - 1]) = nullptr;
      }
      void draw(Tilemap & tilemap, Rect2i viewport) {
      }
      void tick() {
      }

      private:
      struct Particle {
        Vec2f pos;
        Vec2f dir;
        Color color;
      };

      Particle * particles = nullptr;
      Particle * next_free = nullptr;

      Particle * alloc_particle() {
        if(next_free) {
          Particle * p = next_free;
          next_free = *((Particle **)next_free);
          new (p) Particle;
          return p;
        } else {
          return nullptr;
        }
      }
      void free_particle(Particle * p) {
        p->~Particle();
        *((Particle **)p) = next_free;
        next_free = p;
      }
    };
    */

    static Tilemap::Color to_tilemap_color(Color src) {
      return Tilemap::Color(round(src.r * 255),
                            round(src.g * 255),
                            round(src.b * 255));
    }
    static void add_color(Tilemap::Color & dst, Tilemap::Color amt) {
      Tilemap::Color dst_prev = dst;
      dst += amt;
      if(dst.r < dst_prev.r) { dst.r = 255; };
      if(dst.g < dst_prev.g) { dst.g = 255; };
      if(dst.b < dst_prev.b) { dst.b = 255; };
    }

    class ParticleSystem {
      public:
      void start(Vec2i pos) {
        for(int i = 0 ; i < 10 ; i ++) {
          Particle p;
          p.pos = pos;
          float speed = (float)rand() / RAND_MAX * 0.1f;
          float theta = (float)rand() / RAND_MAX * 2.0f * 3.14159f;
          p.dpos = Vec2f(speed*cos(theta), speed*sin(theta));
          p.bg_color = Color(0.4f, 0.4f, 0.4f);
          p.bg_colormode = Particle::SET;
          p.life = (rand() % 15) + 5;
          particles.push_back(p);
        }
        for(int i = 0 ; i < 20 ; i ++) {
          Particle p;
          p.pos = pos;
          float speed = (float)rand() / RAND_MAX * 0.5f;
          float theta = (float)rand() / RAND_MAX * 2.0f * 3.14159f;
          p.dpos = Vec2f(speed*cos(theta), speed*sin(theta));
          p.bg_color = Color(0.2f, 0.1f, 0.0f);
          p.bg_colormode = Particle::ADD;
          p.fg_color = Color(0.5f, 0.5f, 0.5f);
          p.fg_colormode = Particle::SET;
          p.life = (rand() % 10) + 5;
          particles.push_back(p);
        }
      }
      void draw(Tilemap & tilemap, Rect2i viewport) {
        for(auto & p : particles) {
          Vec2i pos = Vec2i(round(p.pos.x), round(p.pos.y)) - viewport.pos;
          if(tilemap.tiles.valid(pos)) {
            if(p.fg_colormode == Particle::ADD) {
              add_color(tilemap.tiles[pos].foreground_color, to_tilemap_color(p.fg_color));
            } else if(p.fg_colormode == Particle::SET) {
              tilemap.tiles[pos].foreground_color = to_tilemap_color(p.fg_color);
            }

            if(p.bg_colormode == Particle::ADD) {
              add_color(tilemap.tiles[pos].background_color, to_tilemap_color(p.bg_color));
            } else if(p.bg_colormode == Particle::SET) {
              tilemap.tiles[pos].background_color = to_tilemap_color(p.bg_color);
            }
          }
        }
      }
      void tick() {
        for(auto it = particles.begin() ;
            it != particles.end() ; ) {
          if(it->life == 0) {
            it = particles.erase(it);
          } else {
            it ++;
          }
        }
        for(auto & p : particles) {
          p.life --;
          p.pos += p.dpos;
        }
      }
      bool finished() {
        return particles.empty();
      }

      private:
      struct Particle {
        Vec2f pos;
        Vec2f dpos;
        Color fg_color;
        Color bg_color;
        unsigned int glyph_index;
        unsigned int life = 0;
        enum ColorMode { IGNORE, ADD, SET };
        ColorMode fg_colormode = IGNORE;
        ColorMode bg_colormode = IGNORE;
      };
      std::vector<Particle> particles;
    };
  }
}

#endif
