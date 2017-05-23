
#include "FOV.hpp"

#include <cstdio>
#include <algorithm>

std::vector<Vec2i> FOV::sample_sparse(const Rect2i & rect) const {
  int xstart = rect.pos.x;
  int ystart = rect.pos.y;
  int xend = rect.pos.x + rect.size.x;
  int yend = rect.pos.y + rect.size.y;

  std::vector<Vec2i> result;
  for(int y = ystart ; y < yend ; y ++) {
    for(int x = xstart ; x < xend ; x ++) {
      Vec2i pos(x, y);
      if(is_visible(pos)) {
        result.push_back(pos);
      }
    }
  }
  return result;
}
std::vector<bool> FOV::sample(const Rect2i & rect) const {
  int xstart = rect.pos.x;
  int ystart = rect.pos.y;
  int xend = rect.pos.x + rect.size.x;
  int yend = rect.pos.y + rect.size.y;

  std::vector<bool> result(rect.size.x * rect.size.y);

  for(int y = ystart ; y < yend ; y ++) {
    for(int x = xstart ; x < xend ; x ++) {
      Vec2i pos(x, y);
      result[x + y * rect.size.x] = is_visible(pos);
    }
  }

  return result;
}

static float diagonal_len(Vec2i ray) {
  const float D = 1.0f;
  const float D2 = 1.414f;
  float dx = std::abs(ray.x);
  float dy = std::abs(ray.y);
  return D * (dx + dy) + (D2 - 2 * D) * std::min(dx, dy);
}

void BresenhamFOV::raycast_o0(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray) {
  int j = 0, e = 0;

  for(int i = 0 ; i <= ray.x ; i ++) {
    values[_radius + i + (_radius + j)*_width] = visible_value;
    if(solid_map.get(p + Vec2i(i, j)) == 1) { return; }

    if(diagonal_len(Vec2i(i, j)) > _radius) { return; }

    e += ray.y;
    if(2*e >= ray.x) {
      j ++;
      e -= ray.x;
    }
  }
}
void BresenhamFOV::raycast_o1(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray) {
  int j = 0, e = 0;

  for(int i = 0 ; i <= ray.x ; i ++) {
    values[_radius + j + (_radius + i)*_width] = visible_value;
    if(solid_map.get(p + Vec2i(j, i)) == 1) { return; }

    if(diagonal_len(Vec2i(i, j)) > _radius) { return; }

    e += ray.y;
    if(2*e >= ray.x) {
      j ++;
      e -= ray.x;
    }
  }
}
void BresenhamFOV::raycast_o2(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray) {
  int j = 0, e = 0;

  for(int i = 0 ; i <= ray.x ; i ++) {
    values[_radius - j + (_radius + i)*_width] = visible_value;
    if(solid_map.get(p + Vec2i(-j, i)) == 1) { return; }

    if(diagonal_len(Vec2i(i, j)) > _radius) { return; }

    e += ray.y;
    if(2*e >= ray.x) {
      j ++;
      e -= ray.x;
    }
  }
}
void BresenhamFOV::raycast_o3(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray) {
  int j = 0, e = 0;

  for(int i = 0 ; i <= ray.x ; i ++) {
    values[_radius - i + (_radius + j)*_width] = visible_value;
    if(solid_map.get(p + Vec2i(-i, j)) == 1) { return; }

    if(diagonal_len(Vec2i(i, j)) > _radius) { return; }

    e += ray.y;
    if(2*e >= ray.x) {
      j ++;
      e -= ray.x;
    }
  }
}
void BresenhamFOV::raycast_o4(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray) {
  int j = 0, e = 0;

  for(int i = 0 ; i <= ray.x ; i ++) {
    values[_radius - i + (_radius - j)*_width] = visible_value;
    if(solid_map.get(p + Vec2i(-i, -j)) == 1) { return; }

    if(diagonal_len(Vec2i(i, j)) > _radius) { return; }

    e += ray.y;
    if(2*e >= ray.x) {
      j ++;
      e -= ray.x;
    }
  }
}
void BresenhamFOV::raycast_o5(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray) {
  int j = 0, e = 0;

  for(int i = 0 ; i <= ray.x ; i ++) {
    values[_radius - j + (_radius - i)*_width] = visible_value;
    if(solid_map.get(p + Vec2i(-j, -i)) == 1) { return; }

    if(diagonal_len(Vec2i(i, j)) > _radius) { return; }

    e += ray.y;
    if(2*e >= ray.x) {
      j ++;
      e -= ray.x;
    }
  }
}
void BresenhamFOV::raycast_o6(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray) {
  int j = 0, e = 0;

  for(int i = 0 ; i <= ray.x ; i ++) {
    values[_radius + j + (_radius - i)*_width] = visible_value;
    if(solid_map.get(p + Vec2i(j, -i)) == 1) { return; }

    if(diagonal_len(Vec2i(i, j)) > _radius) { return; }

    e += ray.y;
    if(2*e >= ray.x) {
      j ++;
      e -= ray.x;
    }
  }
}
void BresenhamFOV::raycast_o7(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray) {
  int j = 0, e = 0;

  for(int i = 0 ; i <= ray.x ; i ++) {
    values[_radius + i + (_radius - j)*_width] = visible_value;
    if(solid_map.get(p + Vec2i(i, -j)) == 1) { return; }

    if(diagonal_len(Vec2i(i, j)) > _radius) { return; }

    e += ray.y;
    if(2*e >= ray.x) {
      j ++;
      e -= ray.x;
    }
  }
}


void BresenhamFOV::update(const Map<unsigned int> & solid_map, Vec2i p, unsigned int r) {
  _width = r*2 + 1;
  _radius = r;
  _origin = p - Vec2i(r, r);

  if(values.size() != _width * _width) {
    values.resize(_width * _width, visible_value);
  }
  visible_value ++;

  for(int y = 0 ; y <= r ; y ++) {
    int x = r;
    raycast_o0(solid_map, p, Vec2i(x, y));
    raycast_o1(solid_map, p, Vec2i(x, y));
    raycast_o2(solid_map, p, Vec2i(x, y));
    raycast_o3(solid_map, p, Vec2i(x, y));
    raycast_o4(solid_map, p, Vec2i(x, y));
    raycast_o5(solid_map, p, Vec2i(x, y));
    raycast_o6(solid_map, p, Vec2i(x, y));
    raycast_o7(solid_map, p, Vec2i(x, y));
  }
}

bool BresenhamFOV::is_visible(Vec2i pos) const {
  int i = pos.x - _origin.x;
  int j = pos.y - _origin.y;

  if(i < 0) { return false; }
  if(i >= _width) { return false; }

  if(j < 0) { return false; }
  if(j >= _width) { return false; }

  return values[i + j * _width] == visible_value;
}

