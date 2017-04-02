
namespace mytime {
  static uint32_t _millis() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return (((spec.tv_sec*1000) & 0xFFFFFFFFu) + spec.tv_nsec/1000000) & 0xFFFFFFFFu;
  }
  static uint32_t _millis_cpu() {
    struct timespec spec;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &spec);
    return (((spec.tv_sec*1000) & 0xFFFFFFFFu) + spec.tv_nsec/1000000) & 0xFFFFFFFFu;
  }

  static uint32_t initial_millis = 0;
  static uint32_t initial_millis_cpu = 0;

  void init() {
    initial_millis = _millis();
    initial_millis_cpu = _millis_cpu();
  }

  uint32_t millis() {
    return _millis() - initial_millis;
  }
  uint32_t millis_cpu() {
    return _millis_cpu() - initial_millis_cpu;
  }

  int32_t diff(uint32_t a, uint32_t b) {
    return (int32_t)(a - b);
  }
}

