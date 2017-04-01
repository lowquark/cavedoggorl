#ifndef DRAW_HPP
#define DRAW_HPP

#include "Entity.hpp"
#include "Color.hpp"

static constexpr int TILE_WIDTH  = 32;
static constexpr int TILE_HEIGHT = 32;

static constexpr float WALL_WIDTH  = 28;
static constexpr float WALL_HEIGHT = 28;

static constexpr float FLOOR_DOT_WIDTH  = 4;
static constexpr float FLOOR_DOT_HEIGHT = 4;

static constexpr float AGENT_WIDTH  = 12;
static constexpr float AGENT_HEIGHT = 12;

static constexpr int TILES_X = 20;
static constexpr int TILES_Y = 16;
static constexpr int WINDOW_WIDTH  = TILE_WIDTH * TILES_X;
static constexpr int WINDOW_HEIGHT = TILE_HEIGHT * TILES_Y;

static constexpr bool DRAW_PATHS = false;

static const Color WALL_COLOR(0.7f, 0.7f, 0.75f);
static const Color FLOOR_COLOR(0.3f, 0.3f, 0.3f);

namespace draw {
  extern Vec2f camera_pos;

  void draw_wall(Vec2i pos);
  void draw_floor(Vec2i pos);

  void draw_agent(Vec2i pos, const Color & color);
  void draw_agent(Vec2f pos, const Color & color);
  void draw_path(const std::vector<Vec2i> & path, const Color & color);
}

#endif
