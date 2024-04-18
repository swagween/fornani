
#pragma once
#include <algorithm>

namespace util {

class Cooldown {
  public:
	constexpr void start(int time) { decrementor = time; }
	constexpr void update() { decrementor = std::clamp(--decrementor, 0, INT_FAST16_MAX); }
	constexpr void cancel() { decrementor = 0; }
	[[nodiscard]] auto is_complete() const -> bool { return decrementor == 0; }
	[[nodiscard]] auto running() const -> bool { return decrementor != 0; }
	[[nodiscard]] auto get_cooldown() const -> int { return decrementor; }

  private:
	int decrementor{};
};

} // namespace util