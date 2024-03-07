
#pragma once
#include <algorithm>

namespace util {

class Cooldown {
  public:
	constexpr void start(int time) { decrementor = time; }
	constexpr void update() { decrementor = std::clamp(--decrementor, 0, INT_FAST16_MAX); }
	constexpr bool is_complete() const { return decrementor == 0; }
	constexpr int get_cooldown() const { return decrementor; }

  private:
	int decrementor{};
};

} // namespace util