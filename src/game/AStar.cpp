
#include "AStar.hpp"

#include <cmath>
#include <cassert>
#include <limits>
#include <algorithm>

struct AStarNode {
  Vec2i pos;
  AStarNode * from = nullptr;
  unsigned int gScore = std::numeric_limits<decltype(gScore)>::max();
  float fScore = std::numeric_limits<decltype(fScore)>::infinity();
  bool open = false;
  bool closed = false;
  unsigned int cost = 0;
};

static float manhattan_heuristic(const Vec2i & a, const Vec2i & b) {
  const float D = 1.0f;
  float dx = abs(a.x - b.x);
  float dy = abs(a.y - b.y);
  return D * (dx + dy);
}
static float diagonal_heuristic(const Vec2i & a, const Vec2i & b) {
  const float D = 1.0f;
  const float D2 = sqrt(2);
  float dx = abs(a.x - b.x);
  float dy = abs(a.y - b.y);
  return D * (dx + dy) + (D2 - 2 * D) * std::min(dx, dy);
}

static Vec2i pos_select_8(Vec2i center, int p) {
  assert(p >= 0);
  assert(p < 8);

  switch(p) {
    case 0:
      return center + Vec2i( 0,  1);
    case 1:
      return center + Vec2i( 1,  1);
    case 2:
      return center + Vec2i( 1,  0);
    case 3:
      return center + Vec2i( 1, -1);
    case 4:
      return center + Vec2i( 0, -1);
    case 5:
      return center + Vec2i(-1, -1);
    case 6:
      return center + Vec2i(-1,  0);
    case 7:
      return center + Vec2i(-1,  1);
    default:
      return center;
  }
}
static Vec2i pos_select_4(Vec2i center, int p) {
  assert(p >= 0);
  assert(p < 4);

  switch(p) {
    case 0:
      return center + Vec2i( 0,  1);
    case 1:
      return center + Vec2i( 1,  0);
    case 2:
      return center + Vec2i( 0, -1);
    case 3:
      return center + Vec2i(-1,  0);
    default:
      return center;
  }
}

bool DoAStar4(std::vector<Vec2i> & path_out,
              const Map<unsigned int> & cost_map,
              Vec2i start_pos,
              Vec2i end_pos) {
  Map<AStarNode> nodes(cost_map.w(), cost_map.h());

  for(unsigned int j = 0 ; j < nodes.h() ; j ++) {
    for(unsigned int i = 0 ; i < nodes.w() ; i ++) {
      Vec2i tile_pos = Vec2i(i, j);
      AStarNode node;
      node.pos = tile_pos;
      node.from = nullptr;
      node.gScore = std::numeric_limits<decltype(AStarNode::gScore)>::max();
      node.fScore = std::numeric_limits<decltype(AStarNode::fScore)>::infinity();
      node.open = false;
      node.closed = false;
      node.cost = cost_map.get(tile_pos, std::numeric_limits<unsigned int>::max());
      nodes.set(tile_pos, node);
    }
  }

  path_out.clear();

  AStarNode * end = nodes.get_ptr(end_pos);
  AStarNode * start = nodes.get_ptr(start_pos);

  start->gScore = 0;
  start->fScore = manhattan_heuristic(start_pos, end_pos);

  std::vector<AStarNode *> open_set;
  start->open = true;
  open_set.push_back(start);

  while(open_set.size()) {
    float min_fScore = std::numeric_limits<float>::infinity();
    auto min_open_it = open_set.begin();

    for(auto it = open_set.begin() ;
        it != open_set.end() ; 
        it ++) {
      if((*it)->fScore < min_fScore) {
        min_fScore = (*it)->fScore;
        min_open_it = it;
      }
    }

    auto current = *min_open_it;

    if(current == end) {
      auto prev = end;
      while(prev) {
        path_out.push_back(prev->pos);
        prev = prev->from;
      }
      return true;
    }

    open_set.erase(min_open_it);
    current->open = false;
    current->closed = true;
    // There doesn't need to be a closed_set
    //closed_set.push_back(current);

    for(int i = 0 ; i < 4 ; i ++) {
      auto neighbor = nodes.get_ptr(pos_select_4(current->pos, i));

      if(!neighbor) { continue; }

      if(neighbor->closed) { continue; }

      unsigned int gScore_tenative = current->gScore + neighbor->cost;

      if(!neighbor->open) {
        neighbor->open = true;
        open_set.push_back(neighbor);
      } else if(gScore_tenative >= neighbor->gScore) {
        continue;
      }

      neighbor->from = current;
      neighbor->gScore = gScore_tenative;
      neighbor->fScore = gScore_tenative + manhattan_heuristic(neighbor->pos, end_pos);
    }
  }

  return false;
}

bool DoAStar8(std::vector<Vec2i> & path_out,
              const Map<unsigned int> & cost_map,
              Vec2i start_pos,
              Vec2i end_pos) {
  Map<AStarNode> nodes(cost_map.w(), cost_map.h());

  for(unsigned int j = 0 ; j < nodes.h() ; j ++) {
    for(unsigned int i = 0 ; i < nodes.w() ; i ++) {
      Vec2i tile_pos = Vec2i(i, j);
      AStarNode node;
      node.pos = tile_pos;
      node.from = nullptr;
      node.gScore = std::numeric_limits<decltype(AStarNode::gScore)>::max();
      node.fScore = std::numeric_limits<decltype(AStarNode::fScore)>::infinity();
      node.open = false;
      node.closed = false;
      node.cost = cost_map.get(tile_pos, std::numeric_limits<unsigned int>::max());
      nodes.set(tile_pos, node);
    }
  }

  path_out.clear();

  AStarNode * end = nodes.get_ptr(end_pos);
  AStarNode * start = nodes.get_ptr(start_pos);

  start->gScore = 0;
  start->fScore = diagonal_heuristic(start_pos, end_pos);

  std::vector<AStarNode *> open_set;
  start->open = true;
  open_set.push_back(start);

  while(open_set.size()) {
    float min_fScore = std::numeric_limits<float>::infinity();
    auto min_open_it = open_set.begin();

    for(auto it = open_set.begin() ;
        it != open_set.end() ; 
        it ++) {
      if((*it)->fScore < min_fScore) {
        min_fScore = (*it)->fScore;
        min_open_it = it;
      }
    }

    auto current = *min_open_it;

    if(current == end) {
      auto prev = end;
      while(prev) {
        path_out.push_back(prev->pos);
        prev = prev->from;
      }
      return true;
    }

    open_set.erase(min_open_it);
    current->open = false;
    current->closed = true;
    // There doesn't need to be a closed_set
    //closed_set.push_back(current);

    for(int i = 0 ; i < 8 ; i ++) {
      auto neighbor = nodes.get_ptr(pos_select_8(current->pos, i));

      if(!neighbor) { continue; }

      if(neighbor->closed) { continue; }

      unsigned int gScore_tenative = current->gScore + neighbor->cost;

      if(!neighbor->open) {
        neighbor->open = true;
        open_set.push_back(neighbor);
      } else if(gScore_tenative >= neighbor->gScore) {
        continue;
      }

      neighbor->from = current;
      neighbor->gScore = gScore_tenative;
      neighbor->fScore = gScore_tenative + diagonal_heuristic(neighbor->pos, end_pos);
    }
  }

  return false;
}

