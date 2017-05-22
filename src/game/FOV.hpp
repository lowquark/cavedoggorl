#ifndef GAME_FOV_HPP
#define GAME_FOV_HPP

#include <vector>

#include <util/Vec2.hpp>
#include <util/Map.hpp>

class FOV {
  public:
  virtual bool is_visible(Vec2i pos) const { return false; }
};

class GlobalFOV : public FOV {
  public:
  bool is_visible(Vec2i pos) const override { return true; }
};

class BresenhamFOV : public FOV {
  public:
  void update(const Map<unsigned int> & solid_map, Vec2i p, unsigned int r);

  bool is_visible(Vec2i pos) const override;

  unsigned int width() const { return _width; }
  unsigned int radius() const { return _radius; }
  Vec2i origin() const { return _origin; }

  private:
  unsigned int _width = 0;
  unsigned int _radius = 0;
  Vec2i _origin;

  std::vector<unsigned int> values;
  unsigned int visible_value = 0;

  void raycast_o0(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray);
  void raycast_o1(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray);
  void raycast_o2(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray);
  void raycast_o3(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray);
  void raycast_o4(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray);
  void raycast_o5(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray);
  void raycast_o6(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray);
  void raycast_o7(const Map<unsigned int> & solid_map, Vec2i p, Vec2i ray);
};

#endif
