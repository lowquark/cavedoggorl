#ifndef ASTAR_HPP
#define ASTAR_HPP

#include <vector>

#include <util/Vec2.hpp>
#include <util/Map.hpp>

bool DoAStar8(std::vector<Vec2i> & path_out,
              const Map<unsigned int> & cost_map,
              Vec2i start_pos,
              Vec2i end_pos);

bool DoAStar4(std::vector<Vec2i> & path_out,
              const Map<unsigned int> & cost_map,
              Vec2i start_pos,
              Vec2i end_pos);

#endif
