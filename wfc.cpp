
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <algorithm>
#include <queue>
#include <vector>

typedef unsigned int StateId;

class StateSpace {
  std::vector<bool> state_probs;
  unsigned int _num_possible = 0;

  public:
  bool has_one_of(const std::vector<int> & states) const {
    for(auto & state_id : states) {
      assert(state_id < state_probs.size());
      assert(state_id >= 0);
      if(state_probs[state_id]) {
        return true;
      }
    }
    return false;
  }

  bool is_possible(int state_id) const {
    assert(state_id < state_probs.size());
    assert(state_id >= 0);
    return state_probs[state_id];
  }
  void set_impossible(int state_id) {
    assert(state_id < state_probs.size());
    assert(state_id >= 0);
    if(state_probs[state_id]) {
      _num_possible --;
      state_probs[state_id] = false;
    }
  }

  void reset(int n_states) {
    assert(n_states >= 0);
    state_probs.resize(n_states, true);
    _num_possible = n_states;
  }

  unsigned int num_possible() const {
    /*
    unsigned int n = 0;
    for(auto prob : state_probs) {
      if(prob) {
        n ++;
      }
    }
    assert(n == _num_possible);
    */
    return _num_possible;
  }

  int choose() {
    unsigned int sum = num_possible();

    if(sum > 0) {
      unsigned int r = rand() % sum;
      for(unsigned int i = 0 ; i < state_probs.size() ; i ++) {
        if(state_probs[i]) {
          if(r == 0) {
            std::fill(state_probs.begin(), state_probs.end(), false);
            state_probs[i] = true;
            return i;
          }
          r --;
        }
      }
    }

    return -1;
  }
};

struct Element {
  unsigned int x;
  unsigned int y;
  const StateSpace * space;

  Element(unsigned int x, unsigned int y, const StateSpace & space)
    : x(x)
    , y(y)
    , space(&space) {}

  bool operator<(const Element & other) const {
    return space->num_possible() > other.space->num_possible();
  }
};

struct AdjacencyGraph {
  std::vector<std::vector<bool>> flags_array;

  public:
  void clear() {
    flags_array.clear();
  }
  void set_size(unsigned int size) {
    flags_array.resize(size);
    for(auto & flags : flags_array) {
      flags.resize(size, false);
    }
  }

  bool get(unsigned int a, unsigned int b) const {
    assert(a < flags_array.size());
    assert(b < flags_array.size());

    return flags_array[a][b];
  }
  void set(unsigned int a, unsigned int b) {
    assert(a < flags_array.size());
    assert(b < flags_array.size());

    flags_array[a][b] = true;
    flags_array[b][a] = true;
  }
  void unset(unsigned int a, unsigned int b) {
    assert(a < flags_array.size());
    assert(b < flags_array.size());

    flags_array[a][b] = false;
    flags_array[b][a] = false;
  }
};


std::vector<Element> still_undefined;

unsigned int output_width = 10;
unsigned int output_height = 10;
unsigned int num_states = 0;

unsigned int window_width = 2;
unsigned int window_height = 2;
unsigned int mask_width = window_width * 2 - 1;
unsigned int mask_height = window_height * 2 - 1;

std::vector<std::vector<std::vector<int>>> constraints;

std::vector<StateSpace> state_spaces;
std::vector<int> states;
std::vector<bool> dirty_map;

// Constrains the possible states of `target` to conform to `invariant`
// Returns true if `target` was modified
bool solve(StateSpace & target, const StateSpace & invariant, int dx, int dy) {
  bool modified = false;

  assert(dx + (int)window_width - 1 >= 0);
  assert(dy + (int)window_height - 1 >= 0);
  assert(dx < (int)window_width);
  assert(dy < (int)window_height);

  unsigned int i = dx + window_width - 1;
  unsigned int j = dy + window_height - 1;

  auto & constraint_array = constraints[i + j * mask_width];

  for(int state_id = 0 ; state_id < num_states ; state_id ++) {
    if(target.is_possible(state_id)) {
      // in order for `target` to take this state, the invariant state must
      // be able to take one of thse states
      auto & must_have_array = constraint_array[state_id];

      if(invariant.has_one_of(must_have_array)) {
        // there's still a chance that this value can be taken
        continue;
      } else {
        // based on the states `invariant` may take, it is impossible for
        // `target` to take this state
        target.set_impossible(state_id);
        modified = true;
      }
    }
  }

  return modified;
}

// Constrains all surrounding nodes (within 2*window_size - 1) to the node at the given location
bool propagate_info(unsigned int src_x, unsigned int src_y) {
  bool modified = false;

  assert(src_x < output_width);
  assert(src_y < output_height);

  assert(window_width > 0);
  assert(window_height > 0);

  unsigned int xmin, xmax;
  unsigned int ymin, ymax;

  if(src_x < window_width - 1) {
    xmin = 0;
  } else {
    xmin = src_x - window_width + 1;
  }

  if(src_x > output_width - window_width) {
    xmax = output_width;
  } else {
    xmax = src_x + window_width;
  }

  if(src_y < window_height - 1) {
    ymin = 0;
  } else {
    ymin = src_y - window_height + 1;
  }

  if(src_y > output_height - window_height) {
    ymax = output_height;
  } else {
    ymax = src_y + window_height;
  }

  auto & this_space = state_spaces[src_x + src_y * output_width];

  for(unsigned int y = ymin ; y < ymax ; y ++) {
    for(unsigned int x = xmin ; x < xmax ; x ++) {
      // skip if this is our current position
      if(x == src_x && y == src_y) {
        continue;
      }

      unsigned int output_index = x + y * output_width;

      auto & other_space = state_spaces[output_index];

      if(solve(other_space, this_space, x - src_x, y - src_y)) {
        // Something changed, we'll need to propagate this later
        dirty_map[output_index] = true;
        modified = true;
      }
    }
  }

  return modified;
}
void propagate_info() {
  bool continue_signal = true;

  while(continue_signal) {
    continue_signal = false;

    for(unsigned int y = 0 ; y < output_height ; y ++) {
      for(unsigned int x = 0 ; x < output_width ; x ++) {
        unsigned int output_index = x + y * output_width;

        if(dirty_map[output_index]) {
          dirty_map[output_index] = false;
          if(propagate_info(x, y)) {
            continue_signal = true;
          }
        }
      }
    }
  }
}

unsigned int last_x = 0, last_y = 0;
bool go() {
  srand(time(0));

  while(still_undefined.size()) {
    std::sort(still_undefined.begin(), still_undefined.end());

    auto elem = still_undefined.back();
    still_undefined.pop_back();

    unsigned int output_idx = elem.x + elem.y * output_width;

    auto & state_space = state_spaces[output_idx];
    auto & state = states[output_idx];

    int chosen_state = state_space.choose();
    if(chosen_state == -1) {
      // fuck.
      last_x = elem.x;
      last_y = elem.y;
      return false;
    }
    state = chosen_state;
    dirty_map[elem.x + elem.y * output_width] = true;

    propagate_info();
  }

  return true;
}

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
};

void print_wave(const std::vector<int> & wave, unsigned int size_x, unsigned int size_y) {
  for(unsigned int y = 0 ; y < size_y ; y ++) {
    for(unsigned int x = 0 ; x < size_x ; x ++) {
      int datum = wave[x + y * size_x];
      printf("%c ", datum);
    }
    printf("\n");
  }
}

void print_samples(const std::vector<Sample> & samples) {
  for(unsigned int state_id = 0 ; state_id < num_states ; state_id ++) {
    auto & sample = samples[state_id];

    printf("%02d: ", state_id);
    for(unsigned int wj = 0 ; wj < window_height ; wj ++) {
      for(unsigned int wi = 0 ; wi < window_width ; wi ++) {
        printf("%c ", sample.data[wi + wj*window_width]);
      }
      printf("\n    ");
    }
    printf("\n");
  }
}

void print_states(const std::vector<int> & states, unsigned int size_x, unsigned int size_y) {
  for(unsigned int y = 0 ; y < size_y ; y ++) {
    for(unsigned int x = 0 ; x < size_x ; x ++) {
      int state = states[x + y * size_x];
      if(state == -1) {
        printf(" . ");
      } else {
        printf("%02d ", state);
      }
    }
    printf("\n");
  }
}
void print_states(const std::vector<int> & states, unsigned int size_x, unsigned int size_y, unsigned int cursor_x, unsigned int cursor_y) {
  for(unsigned int y = 0 ; y < size_y ; y ++) {
    for(unsigned int x = 0 ; x < size_x ; x ++) {
      if(x == cursor_x && y == cursor_y) {
        printf(" * ");
      } else {
        int state = states[x + y * size_x];
        if(state == -1) {
          printf(" . ");
        } else {
          printf("%02d ", state);
        }
      }
    }
    printf("\n");
  }
}

void print_output(const std::vector<int> & states, const std::vector<Sample> & samples, unsigned int size_x, unsigned int size_y) {
  for(unsigned int y = 0 ; y < size_y ; y ++) {
    for(unsigned int x = 0 ; x < size_x ; x ++) {
      int state = states[x + y * size_x];
      auto & sample = samples[state];
      printf("%c ", sample.data[0]);
    }
    printf("\n");
  }
}

int main() {
  std::vector<int> wave {
    '-', '-',
    '-', '*',
  };
  unsigned int wave_size_x = 2;
  unsigned int wave_size_y = 2;

  int num_states_x = wave_size_x - window_width + 1;
  int num_states_y = wave_size_y - window_height + 1;

  printf("%d, %d\n", num_states_x, num_states_y);

  std::vector<Sample> samples;
  for(int j = 0 ; j < num_states_y ; j ++) {
    for(int i = 0 ; i < num_states_x ; i ++) {
      unsigned int id = samples.size();
      samples.emplace_back();
      samples.back().set(i, j, window_width, window_height, wave, wave_size_x);
      samples.back().id = id;

      id = samples.size();
      samples.emplace_back();
      samples.back().set_mirror_x(i, j, window_width, window_height, wave, wave_size_x);
      samples.back().id = id;

      id = samples.size();
      samples.emplace_back();
      samples.back().set_mirror_y(i, j, window_width, window_height, wave, wave_size_x);
      samples.back().id = id;

      id = samples.size();
      samples.emplace_back();
      samples.back().set_mirror_xy(i, j, window_width, window_height, wave, wave_size_x);
      samples.back().id = id;
    }
  }

  num_states = samples.size();

  constraints.resize(mask_width * mask_height);
  for(auto & constraint_array : constraints) {
    constraint_array.resize(num_states);
  }

  // For every combination of samples
  for(auto & sample_a : samples) {
    for(auto & sample_b : samples) {
      // For every possible overlay
      //  dx on [ 1 - window_width, window_width - 1]
      //   i on [ 0, window_width * 2 - 1 ) or [ 0, mask_width )
      //  dy on [ 1 - window_height, window_height - 1]
      //   j on [ 0, window_height * 2 - 1 ) or [ 0, mask_height )
      for(unsigned int j = 0 ; j < mask_width ; j ++) {
        for(unsigned int i = 0 ; i < mask_width ; i ++) {
          auto & constraint_array = constraints[i + j * mask_width];
          auto & must_have_array = constraint_array[sample_b.id];

          int dx = i - window_width + 1;
          int dy = j - window_height + 1;

          if(dx == 0 && dy == 0) {
            continue;
          } else {
            // These two samples match given this overlay, so set their adjacency
            // matrix entries to true
            if(Sample::compatible(sample_a, sample_b, dx, dy)) {
              printf("Match between %u and %u with dx = %d and dy = %d\n", sample_a.id, sample_b.id, dx, dy);
              must_have_array.push_back(sample_a.id);
            }
          }
        }
      }
    }
  }

  state_spaces.resize(output_width * output_height);
  states.resize(output_width * output_height, -1);
  dirty_map.resize(output_width * output_height, false);

  for(auto & state_space : state_spaces) {
    state_space.reset(num_states);
  }

  for(unsigned int y = 0 ; y < output_height ; y ++) {
    for(unsigned int x = 0 ; x < output_width ; x ++) {
      still_undefined.emplace_back(x, y, state_spaces[x + y * output_width]);
    }
  }

  if(go()) {
    printf("-- Wave --\n");
    print_wave(wave, wave_size_x, wave_size_y);

    printf("-- Samples --\n");
    print_samples(samples);

    printf("-- States --\n");
    print_states(states, output_width, output_height);

    printf("-- Output --\n");
    print_output(states, samples, output_width, output_height);
  } else {
    printf("failed.\n");

    printf("-- Wave --\n");
    print_wave(wave, wave_size_x, wave_size_y);

    printf("-- Samples --\n");
    print_samples(samples);

    printf("-- States --\n");
    print_states(states, output_width, output_height, last_x, last_y);

    /*
    int y = (int)last_y + 1 - window_height;
    for(unsigned int j = 0 ; j < mask_height ; ++j) {

      int x = (int)last_x + 1 - window_width;
      for(unsigned int i = 0 ; i < mask_width ; ++i) {
        // skip if this is our current position
        if(!(x == last_x && y == last_y)) {
          int state = states[x + y * output_width];

          if(state != -1) {
            auto & sample = samples[state];
            printf("%02d: ", state);
            for(unsigned int wj = 0 ; wj < window_height ; wj ++) {
              for(unsigned int wi = 0 ; wi < window_width ; wi ++) {
                printf("%c ", sample.data[wi + wj*window_width]);
              }
              printf("\n    ");
            }
            printf("\n");
          }
        }

        ++x;
      }

      ++y;
    }
    */

  }

  return 0;
}

