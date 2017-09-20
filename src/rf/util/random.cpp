
#include "random.hpp"

#include <cassert>
#include <climits>

namespace rf {
  void CMWC4096::State::seed(uint32_t seed) {
    /* choose random initial c<809430660 and */
    /* 4096 random 32-bit integers for Q[]   */

    uint32_t s = seed;
    uint32_t m_bits = UINT32_MAX;
    uint32_t a = 1664525;
    uint32_t c = 1013904223;

    for(int i = 0 ; i < 4096 ; i ++) {
      uint64_t newseed = s;
      newseed *= a;
      newseed += c;
      s = newseed & m_bits;

      Q[i] = s;
    }

    uint64_t newseed = s;
    newseed *= a;
    newseed += c;
    newseed &= m_bits;

    c = newseed * 809430660 / 0x100000000LL;

    // max value of newseed is 0xFFFFFFFF,
    // dividing by 0x100000000 ought to truncate to 809430659 at max
    assert(c < 809430660);

    idx = 4095;
  }

  uint32_t CMWC4096::operator()() {
    const uint64_t a = 18782LL;
    const uint32_t r = 0xfffffffe;

    _state.idx = (_state.idx + 1) & 4095;

    uint64_t t = a * _state.Q[_state.idx] + _state.c;

    _state.c = (t >> 32);

    uint32_t x = t + _state.c;
    if(x < _state.c) {
      x ++;
      _state.c ++;
    }

    return _state.Q[_state.idx] = r - x;
  }

  namespace random {
    // returns a random number uniformly distributed on [0, 1)
    template <>
    double lerp<double>(RNG & gen) {
      return ((double)gen()) / 0x100000000LL;
    }
    // returns a random number uniformly distributed on [0, 1)
    template <>
    float lerp<float>(RNG & gen) {
      return ((float)gen()) / 0x100000000LL;
    }

    // returns a random number uniformly distributed on [min, max)
    template <>
    double range<double>(RNG & gen, double min, double max) {
      return lerp<double>(gen) * (max - min) + min;
    }
    // returns a random number uniformly distributed on [min, max)
    template <>
    float range<float>(RNG & gen, float min, float max) {
      return lerp<float>(gen) * (max - min) + min;
    }
    // returns a random number uniformly distributed on [min, max)
    template <>
    int range<int>(RNG & gen, int min, int max) {
      return ((long long int)gen()) * (max - min) / 0x100000000LL + min;
    }

    // rolls k, n-sided dice and returns the sum
    int dice(RNG & gen, int k, int n) {
      int sum = 0;
      for(int i = 0 ; i < k ; i ++) {
        sum += range<int>(gen, 0, n) + 1;
      }
      return sum;
    }

    unsigned int lottery(RNG & gen, const unsigned int * prob, unsigned int nprob) {
      unsigned int sum = 0;
      for(unsigned int i = 0 ; i < nprob ; i ++) {
        assert(sum + prob[i] > sum); // protect from overflow
        sum += prob[i];
      }

      assert(sum <= INT_MAX);
      int r = range<int>(gen, 0, sum);

      assert(r < sum);

      for(unsigned int i = 0 ; i < nprob ; i ++) {
        r -= prob[i];
        if(r < 0) {
          return i;
        }
      }

      // shouldn't get here
      return 0;
    }
    unsigned int lottery(RNG & gen, const std::vector<unsigned int> & vec) {
      return lottery(gen, vec.data(), vec.size());
    }

    int neighbor(RNG & gen, int i) {
      if(gen() & 0x1) {
        return i + 1;
      } else {
        return i - 1;
      }
    }
    Vec2i neighbor(RNG & gen, Vec2i p) {
      switch(range(gen, 0, 8)) {
        case 0:
          return p + Vec2i(1, 0);
        case 1:
          return p + Vec2i(1, 1);
        case 2:
          return p + Vec2i(0, 1);
        case 3:
          return p + Vec2i(-1, 1);
        case 4:
          return p + Vec2i(-1, 0);
        case 5:
          return p + Vec2i(-1, -1);
        case 6:
          return p + Vec2i(0, -1);
        case 7:
          return p + Vec2i(1, -1);
        default:
          return p;
      }
    }
    Vec2i orthogonal_neighbor(RNG & gen, Vec2i p) {
      switch(range(gen, 0, 4)) {
        case 0:
          return p + Vec2i(1, 0);
        case 1:
          return p + Vec2i(0, 1);
        case 2:
          return p + Vec2i(-1, 0);
        case 3:
          return p + Vec2i(0, -1);
        default:
          return p;
      }
    }
    Vec2i diagonal_neighbor(RNG & gen, Vec2i p) {
      switch(range(gen, 0, 4)) {
        case 0:
          return p + Vec2i( 1,  1);
        case 1:
          return p + Vec2i( 1, -1);
        case 2:
          return p + Vec2i(-1, -1);
        case 3:
          return p + Vec2i(-1,  1);
        default:
          return p;
      }
    }
  }
}

