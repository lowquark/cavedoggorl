#ifndef RF_GAME_ASTAR_HPP
#define RF_GAME_ASTAR_HPP

#include <vector>

#include <rf/util/Vec2.hpp>
#include <rf/util/Map.hpp>

namespace rf {
  bool DoAStar8(std::vector<Vec2i> & path_out,
                const Map<unsigned int> & cost_map,
                Vec2i start_pos,
                Vec2i end_pos);

  bool DoAStar4(std::vector<Vec2i> & path_out,
                const Map<unsigned int> & cost_map,
                Vec2i start_pos,
                Vec2i end_pos);
}

#endif
