#ifndef RF_UTIL_RANDOM_HPP
#define RF_UTIL_RANDOM_HPP

#include <vector>
#include <cstdint>

#include <rf/util/Vec2.hpp>

namespace rf {
  class RNG {
    public:
    virtual ~RNG() = default;
    virtual uint32_t operator()() = 0;
  };

  class CMWC4096 : public RNG {
    public:
    struct State {
      uint32_t Q[4096];
      uint32_t c;
      int idx;

      State() { seed(0); }
      State(uint32_t s) { seed(s); }

      void seed(uint32_t seed);

      // TODO: serializable
    };

    CMWC4096() = default;
    CMWC4096(uint32_t s) : _state(s) {}

    void seed(uint32_t s) {
      _state.seed(s);
    }
    void set_state(const State & state) {
      _state = state;
    }
    State state() const {
      return _state;
    }

    virtual uint32_t operator()() override;

    private:
    State _state;
  };

  namespace random {
    // returns a random number uniformly distributed on [0, 1)
    template <typename T = double>
    T lerp(RNG & gen);
    // returns a random number uniformly distributed on [min, max)
    template <typename T = double>
    T range(RNG & gen, T min, T max);

    // returns a random number uniformly distributed on [0, 1)
    template <>
    double lerp<double>(RNG & gen);
    // returns a random number uniformly distributed on [0, 1)
    template <>
    float lerp<float>(RNG & gen);

    // returns a random number uniformly distributed on [min, max)
    template <>
    double range<double>(RNG & gen, double min, double max);
    // returns a random number uniformly distributed on [min, max)
    template <>
    float range<float>(RNG & gen, float min, float max);
    // returns a random number uniformly distributed on [min, max)
    template <>
    int range<int>(RNG & gen, int min, int max);

    // rolls k, n-sided dice and returns the sum
    int dice(RNG & gen, int k, int n);
    // rolls an n-sided dice
    inline int dice(RNG & gen, int n) { return range<int>(gen, 0, n) + 1; }
    // extreme convenience
    inline int d20(RNG & gen) { return dice(gen, 20); }
    inline int d12(RNG & gen) { return dice(gen, 12); }
    inline int d10(RNG & gen) { return dice(gen, 10); }
    inline int d8(RNG & gen) { return dice(gen, 8); }
    inline int d6(RNG & gen) { return dice(gen, 6); }
    inline int d4(RNG & gen) { return dice(gen, 4); }

    unsigned int lottery(RNG & gen, const unsigned int * prob, unsigned int nprob);
    unsigned int lottery(RNG & gen, const std::vector<unsigned int> & prob);

    int neighbor(RNG & gen, int i);
    Vec2i neighbor(RNG & gen, Vec2i p);
    Vec2i orthogonal_neighbor(RNG & gen, Vec2i p);
    Vec2i diagonal_neighbor(RNG & gen, Vec2i p);
  }
}

#endif
