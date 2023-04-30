#include <chrono>

class Timer {
private:
  std::chrono::high_resolution_clock::time_point _t0, _t1;
public:
  Timer() {};
  void start() {
    _t1=_t0=std::chrono::high_resolution_clock::now();
  }
  void stop() {
    _t1=std::chrono::high_resolution_clock::now();
  }
  double millisecs() {
    return std::chrono::duration_cast<std::chrono::duration<double,std::milli>>(_t1-_t0).count();
  }
  double microsecs() {
    return std::chrono::duration_cast<std::chrono::duration<double,std::micro>>(_t1-_t0).count();
  }
  double seconds() {
    return std::chrono::duration_cast<std::chrono::duration<double>>(_t1-_t0).count();
  }
};
