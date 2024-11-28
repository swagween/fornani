
#pragma once
#include <algorithm>
#include <limits>

namespace util {

class Cooldown {
  public:
	Cooldown() = default;
	Cooldown(int time) : native_time(time) {}
	constexpr void start() { decrementor = native_time; }
	constexpr void start(int time) { decrementor = time; }
	constexpr void update() { decrementor = std::clamp(decrementor - 1, 0, std::numeric_limits<int>::max()); }
	constexpr void reverse() { decrementor = std::clamp(decrementor + 1, 0, native_time); }
	constexpr void cancel() { decrementor = 0; }
	constexpr void nullify() { decrementor = -1; }
	[[nodiscard]] auto started() const -> bool { return decrementor == native_time; }
	[[nodiscard]] auto just_started() const -> bool { return decrementor == native_time - 1; }
	[[nodiscard]] auto is_almost_complete() const -> bool { return decrementor == 1; }
	[[nodiscard]] auto is_complete() const -> bool { return decrementor == 0; }
	[[nodiscard]] auto running() const -> bool { return decrementor != 0; }
	[[nodiscard]] auto halfway() const -> bool { return decrementor <= native_time / 2; }
	[[nodiscard]] auto get_cooldown() const -> int { return decrementor; }
	[[nodiscard]] auto get_normalized() const -> float { return static_cast<float>(decrementor) / static_cast<float>(native_time); }
	[[nodiscard]] auto get_quadratic_normalized() const -> float { return static_cast<float>(decrementor * decrementor) / static_cast<float>(native_time * native_time); }
	[[nodiscard]] auto get_cubic_normalized() const -> float { return static_cast<float>(decrementor * decrementor * decrementor) / static_cast<float>(native_time * native_time * native_time); }
	[[nodiscard]] auto get_native_time() const -> int { return native_time; }
	[[nodiscard]] auto null() const -> bool { return decrementor == -1; }

  private:
	int decrementor{};
	int native_time{};
};

} // namespace util