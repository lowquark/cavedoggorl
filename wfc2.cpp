
#include <cstdio>
#include <ctime>
#include <deque>
#include <cstdlib>
#include <climits>
#include <cassert>
#include <vector>
#include <unordered_map>
#include <unistd.h>

namespace wfc {
  class WaveFunction {
    public:
    WaveFunction() = default;
    ~WaveFunction() {
      delete [] constraint_data;
      constraint_data = nullptr;
    }
    WaveFunction(const WaveFunction & other) = delete;
    WaveFunction & operator=(const WaveFunction & other) = delete;

    void init(unsigned int window_size_x,
              unsigned int window_size_y,
              unsigned int num_states,
              const std::vector<unsigned int> & weights) {
      this->window_size_x = window_size_x;
      this->window_size_y = window_size_y;
      this->num_states = num_states;
      this->weights = weights;

      this->dx_range = window_size_x * 2 - 1;
      this->dy_range = window_size_y * 2 - 1;


      unsigned int constraint_data_len = dx_range * dy_range * num_states * num_states;

      constraint_data = new bool[constraint_data_len];

      for(unsigned int idx = 0 ; idx < constraint_data_len ; idx ++) {
        constraint_data[idx] = false;
      }
    }

    void add_pair(int dx, int dy, unsigned int state_a, unsigned int state_b) {
      unsigned int i0 = dx + window_size_x - 1;
      unsigned int j0 = dy + window_size_y - 1;
      unsigned int i1 = -dx + window_size_x - 1;
      unsigned int j1 = -dy + window_size_y - 1;

      unsigned int idx0 = (i0 + j0 * dx_range)*(num_states*num_states);
      unsigned int idx1 = (i1 + j1 * dx_range)*(num_states*num_states);

      auto adjacency_matrix0 = constraint_data + idx0;
      auto adjacency_matrix1 = constraint_data + idx1;
      adjacency_matrix0[state_a + num_states * state_b] = true;
      adjacency_matrix1[state_b + num_states * state_a] = true;
    }

    private:
    unsigned int window_size_x, window_size_y;
    unsigned int dx_range, dy_range;
    unsigned int num_states;
    std::vector<unsigned int> weights;

    bool * constraint_data = nullptr;

    friend class WaveState;


    bool * adjacency_matrix(int dx, int dy) const {
      unsigned int i = dx + window_size_x - 1;
      unsigned int j = dy + window_size_y - 1;

      unsigned int idx = (i + j * dx_range)*(num_states*num_states);

      return constraint_data + idx;
    }
  };
  class WaveState {
    public:
    WaveState() = default;
    ~WaveState() {
      delete [] state_data;
      state_data = nullptr;
      delete [] dirty_data;
      dirty_data = nullptr;
    }
    WaveState(const WaveFunction & other) = delete;
    WaveState & operator=(const WaveFunction & other) = delete;

    void init(unsigned int size_x, unsigned int size_y, unsigned int num_states) {
      this->size_x = size_x;
      this->size_y = size_y;
      this->num_states = num_states;

      unsigned int state_array_len = (num_states + 1);

      state_data = new unsigned int[size_x * size_y * state_array_len];

      for(unsigned int j = 0 ; j < size_y ; j ++) {
        for(unsigned int i = 0 ; i < size_x ; i ++) {
          unsigned int idx = (i + j * size_x)*state_array_len;
          auto & len = state_data[idx];
          len = num_states;
          for(unsigned int s = 0 ; s < num_states ; s ++) {
            state_data[idx + 1 + s] = s;
          }
        }
      }

      dirty_data = new bool[size_x * size_y];
      for(unsigned int idx = 0 ; idx < size_x * size_y ; idx ++) {
        dirty_data[idx] = false;
      }
    }

    std::vector<unsigned int> possible_states(unsigned int x, unsigned int y) {
      unsigned int state_array_len = (num_states + 1);

      unsigned int idx = (x + y * size_x);
      auto & len = state_data[idx*state_array_len];
      auto data = state_data + idx*state_array_len + 1;

      return std::vector<unsigned int>(data, data + len);
    }

    bool constrain_from(const WaveFunction & function, unsigned int src_x, unsigned int src_y) {
      bool anything_marked_dirty = false;

      unsigned int state_array_len = (num_states + 1);

      unsigned int src_idx = (src_x + src_y * size_x);
      // reference to length entry for possible states of src cell
      auto & src_len = state_data[src_idx*state_array_len];
      auto src_data = state_data + src_idx*state_array_len + 1;

      for(int dy = -(int)function.window_size_y + 1 ; dy < (int)function.window_size_y ; dy ++) {
        for(int dx = -(int)function.window_size_x + 1 ; dx < (int)function.window_size_x ; dx ++) {
          if(dx == 0 && dy == 0) {
            continue;
          }

          auto adjacency_matrix = function.adjacency_matrix(dx, dy);

          // gotta keep that modulo result positive by adding size_x or size_y
          unsigned int dst_x = (src_x + dx + size_x) % size_x;
          unsigned int dst_y = (src_y + dy + size_y) % size_y;

          unsigned int dst_idx = (dst_x + dst_y * size_x);
          // reference to length entry for possible states of dst cell
          auto & dst_len = state_data[dst_idx*state_array_len];
          auto dst_data = state_data + dst_idx*state_array_len + 1;

          // remove impossible states from dst
          for(unsigned int dst_i = 0 ; dst_i < dst_len ; ) {
            unsigned int dst_state = dst_data[dst_i];

            // record for longest bool name
            bool is_dst_state_compatible_with_src_cell = false;

            for(unsigned int src_i = 0 ; src_i < src_len ; src_i ++) {
              unsigned int src_state = src_data[src_i];

              if(adjacency_matrix[src_state + dst_state * function.num_states]) {
                is_dst_state_compatible_with_src_cell = true;
                break;
              }
            }

            if(is_dst_state_compatible_with_src_cell) {
              dst_i ++;
            } else {
              if(dst_len == 1) {
                exit(-1);
                // fuck
              } else {
                // Remove by setting current value to last value,
                // decrementing length
                dst_len --;
                dst_data[dst_i] = dst_data[dst_len];
                // Mark durty
                dirty_data[dst_x + dst_y * size_x] = true;
                anything_marked_dirty = true;
              }
            }
          }
        }
      }

      return anything_marked_dirty;
    }

    void solve(const WaveFunction & function) {
      bool continue_signal = true;

      while(continue_signal) {
        continue_signal = false;

        for(unsigned int y = 0 ; y < size_y ; y ++) {
          for(unsigned int x = 0 ; x < size_x ; x ++) {
            if(dirty_data[x + y * size_x]) {
              dirty_data[x + y * size_x] = false;
              if(constrain_from(function, x, y)) {
                continue_signal = true;
              }
            }
          }
        }
      }
    }

    bool collapse(const WaveFunction & function) {
      unsigned int round_id = 0;

      while(true) {

        unsigned int state_array_len = (num_states + 1);

        unsigned int min_len = UINT_MAX;
        for(unsigned int i = 0 ; i < size_x * size_y ; i ++) {
          unsigned int idx = i * state_array_len;
          auto & len = state_data[idx];

          if(len != 1 && len < min_len) {
            min_len = len;
          }
        }

        if(min_len == UINT_MAX) {
          // everything went better than expected
          // (done solving)
          return true;
        }

        unsigned int n_with_min = 0;
        for(unsigned int i = 0 ; i < size_x * size_y ; i ++) {
          unsigned int idx = i * state_array_len;
          auto & len = state_data[idx];
          if(len == min_len) {
            n_with_min ++;
          }
        }


        unsigned int r_count = rand() % n_with_min;
        unsigned int chosen_idx = 0;
        unsigned int chosen_x = 0;
        unsigned int chosen_y = 0;

        for(unsigned int i = 0 ; i < size_x * size_y ; i ++) {
          unsigned int idx = i * state_array_len;
          auto & len = state_data[idx];
          if(len == min_len) {
            if(r_count == 0) {
              chosen_x = i % size_x;
              chosen_y = i / size_x;
              chosen_idx = idx;
              break;
            } else {
              r_count --;
            }
          }
        }

        // reference to length entry for possible states of this cell
        auto & chosen_len = state_data[chosen_idx];
        auto chosen_data = state_data + chosen_idx + 1;

        if(chosen_len == 0) {
          printf("over-constrained state at %u, %u\n", chosen_x, chosen_y);
          dump();
          return false;
        } else if(chosen_len == 1) {
          printf("how did we get here?");
          return false;
        }

        unsigned int sum_weights = 0;
        for(unsigned int i = 0 ; i < chosen_len ; i ++) {
          unsigned int weight = function.weights[chosen_data[i]];
          sum_weights += weight;
          assert(weight != 0);
        }

        unsigned int chosen_state = UINT_MAX;
        unsigned int randn = rand() % sum_weights;

        for(unsigned int i = 0 ; i < chosen_len ; i ++) {
          unsigned int weight = function.weights[chosen_data[i]];

          if(randn < weight) {
            chosen_state = chosen_data[i];
            break;
          } else {
            randn -= weight;
          }
        }

        assert(chosen_state != UINT_MAX);

        //printf("Chose state %u at %u, %u\n", chosen_state, chosen_x, chosen_y);

        /*
        for(int dy = -(int)function.window_size_y + 1 ; dy < (int)function.window_size_y ; dy ++) {
          for(int dx = -(int)function.window_size_x + 1 ; dx < (int)function.window_size_x ; dx ++) {
            // gotta keep that modulo result positive by adding size_x or size_y
            unsigned int dst_x = (chosen_x + dx + size_x) % size_x;
            unsigned int dst_y = (chosen_y + dy + size_y) % size_y;

            printf("%u, %u (%d, %d):\n", dst_x, dst_y, dx, dy);
            auto possible = possible_states(dst_x, dst_y);
            printf("[ ");
            for(auto & s : possible) {
              printf("%u ", s);
            }
            printf("]\n");
          }
        }
        */

        // make this the only possibility (aka observe this state)
        chosen_len = 1;
        chosen_data[0] = chosen_state;
        // modified, must be marked dirty
        dirty_data[chosen_x + chosen_y * size_x] = true;


        solve(function);

        //dump();

        round_id ++;
      }
    }

    bool is_solved(unsigned int x, unsigned int y) {
      unsigned int state_array_len = (num_states + 1);

      unsigned int idx = (x + y * size_x)*state_array_len;

      auto & len = state_data[idx];

      return len == 1;
    }
    unsigned int get_state(unsigned int x, unsigned int y) {
      unsigned int state_array_len = (num_states + 1);

      unsigned int idx = (x + y * size_x)*state_array_len;

      return state_data[idx + 1];
    }

    void dump() {
      unsigned int state_array_len = (num_states + 1);

      printf("-- Established States --\n");
      for(unsigned int j = 0 ; j < size_y ; j ++) {
        for(unsigned int i = 0 ; i < size_x ; i ++) {
          unsigned int idx = (i + j * size_x)*state_array_len;

          auto & len = state_data[idx];
          auto data = state_data + idx + 1;

          if(len == 1) {
            printf("%02u ", data[0]);
          } else if (len == 0) {
            printf(" _ ");
          } else {
            printf(" * ");
          }
        }
        printf("\n");
      }

      /*
      printf("-- Possibility Table --\n");
      for(unsigned int j = 0 ; j < size_y ; j ++) {
        for(unsigned int i = 0 ; i < size_x ; i ++) {
          printf("%u, %u: [ ", i, j);
          unsigned int idx = (i + j * size_x)*state_array_len;

          auto & len = state_data[idx];
          auto data = state_data + idx + 1;

          for(unsigned int s = 0 ; s < len ; s ++) {
            printf("%u ", data[s]);
          }
          printf("]\n");
        }
      }
      */
    }

    private:
    unsigned int size_x = 0;
    unsigned int size_y = 0;
    unsigned int num_states = 0;

    unsigned int * state_data = nullptr;

    bool * dirty_data = nullptr;
  };

  struct Sample {
    unsigned int id;
    unsigned int x, y;
    unsigned int w, h;
    std::vector<int> data;

    void set(unsigned int x,
             unsigned int y,
             unsigned int w,
             unsigned int h,
             std::vector<int> data,
             unsigned int data_width) {
      this->x = x;
      this->y = y;
      this->w = w;
      this->h = h;

      this->data.resize(w * h);

      for(unsigned int j = 0 ; j < h ; j ++) {
        for(unsigned int i = 0 ; i < w ; i ++) {
          this->data[i + j*w] = data[(x + i) + (y + j) * data_width];
        }
      }
    }

    void set_mirror_x(unsigned int x,
                      unsigned int y,
                      unsigned int w,
                      unsigned int h,
                      std::vector<int> data,
                      unsigned int data_width) {
      this->x = x;
      this->y = y;
      this->w = w;
      this->h = h;

      this->data.resize(w * h);

      for(unsigned int j = 0 ; j < h ; j ++) {
        for(unsigned int i = 0 ; i < w ; i ++) {
          unsigned int src_x = x + w - 1 - i;
          unsigned int dst_x = i;
          unsigned int src_y = y + j;
          unsigned int dst_y = j;
          this->data[dst_x + dst_y*w] = data[src_x + src_y*data_width];
        }
      }
    }

    void set_mirror_y(unsigned int x,
                      unsigned int y,
                      unsigned int w,
                      unsigned int h,
                      std::vector<int> data,
                      unsigned int data_width) {
      this->x = x;
      this->y = y;
      this->w = w;
      this->h = h;

      this->data.resize(w * h);

      for(unsigned int j = 0 ; j < h ; j ++) {
        for(unsigned int i = 0 ; i < w ; i ++) {
          unsigned int src_x = x + i;
          unsigned int dst_x = i;
          unsigned int src_y = y + h - 1 - j;
          unsigned int dst_y = j;
          this->data[dst_x + dst_y*w] = data[src_x + src_y*data_width];
        }
      }
    }

    void set_mirror_xy(unsigned int x,
                       unsigned int y,
                       unsigned int w,
                       unsigned int h,
                       std::vector<int> data,
                       unsigned int data_width) {
      this->x = x;
      this->y = y;
      this->w = w;
      this->h = h;

      this->data.resize(w * h);

      for(unsigned int j = 0 ; j < h ; j ++) {
        for(unsigned int i = 0 ; i < w ; i ++) {
          unsigned int src_x = x + w - 1 - i;
          unsigned int dst_x = i;
          unsigned int src_y = y + h - 1 - j;
          unsigned int dst_y = j;
          this->data[dst_x + dst_y*w] = data[src_x + src_y*data_width];
        }
      }
    }

    static bool compatible(const Sample & a, const Sample & b, int dx, int dy) {
      assert(a.w == b.w);
      assert(a.h == b.h);

      unsigned int xmin, ymin;
      unsigned int xmax, ymax;

      if(dx < 0) {
        xmin = 0;
        xmax = dx + a.w;
      } else {
        xmin = dx;
        xmax = a.w;
      }

      if(dy < 0) {
        ymin = 0;
        ymax = dy + a.h;
      } else {
        ymin = dy;
        ymax = a.h;
      }

      for(unsigned int y = ymin ; y < ymax ; y ++) {
        for(unsigned int x = xmin ; x < xmax ; x ++) {
          if(a.data[x + a.w*y] != b.data[x - dx + b.w*(y - dy)]) {
            return false;
          }
        }
      }

      return true;
    }

    // http://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
    struct Hash {
      std::size_t operator()(const wfc::Sample & sample) const {
        std::size_t seed = sample.data.size();
        for(auto & i : sample.data) {
          seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
      }
    };

    bool operator==(const wfc::Sample & other) const {
      return other.data == data;
    }
  };
}

void print_samples(const std::vector<wfc::Sample> & samples) {
  for(unsigned int state_id = 0 ; state_id < samples.size() ; state_id ++) {
    auto & sample = samples[state_id];

    printf("%02d: ", state_id);
    for(unsigned int wj = 0 ; wj < sample.h ; wj ++) {
      for(unsigned int wi = 0 ; wi < sample.w ; wi ++) {
        printf("%c ", sample.data[wi + wj*sample.w]);
      }
      printf("\n    ");
    }
    printf("\n");
  }
}

int main() {
  std::vector<int> wave {
    '#', '#', '#', '#', 
    '#', '.', '_', '.', 
    '#', '_', '.', '_', 
    '#', '.', '_', '.', 
    '#', '_', '.', '_', 
    '#', '#', '#', '#', 
  };
  unsigned int wave_size_x = 4;
  unsigned int wave_size_y = 6;


  unsigned int window_width = 2;
  unsigned int window_height = 2;

  int num_states_x = wave_size_x - window_width + 1;
  int num_states_y = wave_size_y - window_height + 1;

  std::vector<wfc::Sample> samples;
  std::vector<unsigned int> weights;

  {
    std::unordered_map<wfc::Sample, unsigned int, wfc::Sample::Hash> counts;

    for(int j = 0 ; j < num_states_y ; j ++) {
      for(int i = 0 ; i < num_states_x ; i ++) {
        wfc::Sample sample;

        sample.set(i, j, window_width, window_height, wave, wave_size_x);
        counts[sample] ++;

        sample.set_mirror_x(i, j, window_width, window_height, wave, wave_size_x);
        counts[sample] ++;

        sample.set_mirror_y(i, j, window_width, window_height, wave, wave_size_x);
        counts[sample] ++;

        sample.set_mirror_xy(i, j, window_width, window_height, wave, wave_size_x);
        counts[sample] ++;
      }
    }

    unsigned int id = 0;
    for(auto & kvpair : counts) {
      samples.push_back(kvpair.first);
      samples.back().id = id;

      weights.push_back(kvpair.second);

      printf("state %u has %u counts\n", id, kvpair.second);
      id ++;
    }
  }

  unsigned int num_states = samples.size();

  wfc::WaveFunction wave_function;
  wave_function.init(window_width, window_height, num_states, weights);

  // For every combination of samples
  for(auto & sample_a : samples) {
    for(auto & sample_b : samples) {
      // For every possible overlay
      //  dx on [ 1 - window_width, window_width - 1]
      //   i on [ 0, window_width * 2 - 1 ) or [ 0, mask_width )
      //  dy on [ 1 - window_height, window_height - 1]
      //   j on [ 0, window_height * 2 - 1 ) or [ 0, mask_height )

      for(int dy = -(int)window_height + 1 ; dy < (int)window_height ; dy ++) {
        for(int dx = -(int)window_width + 1 ; dx < (int)window_width ; dx ++) {
          if(dx == 0 && dy == 0) {
            continue;
          } else {
            // These two samples match given this overlay, so set their adjacency
            // matrix entries to true
            if(wfc::Sample::compatible(sample_a, sample_b, dx, dy)) {
              //printf("Match between %u and %u with dx = %d and dy = %d\n", sample_a.id, sample_b.id, dx, dy);

              wave_function.add_pair(dx, dy, sample_a.id, sample_b.id);
            }
          }
        }
      }
    }
  }


  srand(time(0));

  print_samples(samples);


  unsigned int output_size_x = 10;
  unsigned int output_size_y = 10;

  for(int i = 0 ; i < 1 ; i ++) {
    wfc::WaveState wave_state;
    wave_state.init(output_size_x, output_size_y, num_states);

    while(wave_state.collapse(wave_function)) {
      printf("-- Result --\n");
      for(unsigned int j = 0 ; j < output_size_y ; j ++) {
        for(unsigned int i = 0 ; i < output_size_x ; i ++) {
          if(wave_state.is_solved(i, j)) {
            printf("%c ", samples[wave_state.get_state(i, j)].data[0]);
          } else {
            printf("~ ");
          }
        }
        printf("\n");
      }

      wave_state.init(output_size_x, output_size_y, num_states);
    }

    print_samples(samples);
    usleep(100000);
  }

  return 0;
}

