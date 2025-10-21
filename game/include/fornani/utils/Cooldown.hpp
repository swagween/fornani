
#pragma once

#include <ccmath/ext/clamp.hpp>
#include <fornani/utils/Random.hpp>
#include <limits>

namespace fornani::util {

class Cooldown {
  public:
	Cooldown() = default;
	explicit Cooldown(int const time) : native_time(time) {}
	constexpr void start() { decrementor = native_time; }
	constexpr void start(int const time) { decrementor = time; }
	constexpr void update() { decrementor = ccm::ext::clamp(decrementor - 1, 0, std::numeric_limits<int>::max()); }
	constexpr void reverse() { decrementor = ccm::ext::clamp(decrementor + 1, 0, native_time); }
	constexpr void cancel() { decrementor = 0; }
	constexpr void nullify() { decrementor = -1; }
	constexpr void invert() { decrementor = native_time - decrementor; }
	constexpr void set_native_time(int const to) { native_time = to; }
	void randomize() { decrementor = random::random_range(0, native_time); }

	[[nodiscard]] auto started() const -> bool { return decrementor == native_time; }
	[[nodiscard]] auto just_started() const -> bool { return decrementor == native_time - 1; }
	[[nodiscard]] auto is_almost_complete() const -> bool { return decrementor == 1; }
	[[nodiscard]] auto is_complete() const -> bool { return decrementor == 0; }
	[[nodiscard]] auto running() const -> bool { return decrementor != 0; }
	[[nodiscard]] auto halfway() const -> bool { return decrementor <= native_time / 2; }
	[[nodiscard]] auto get() const -> int { return decrementor; }
	[[nodiscard]] auto get_normalized() const -> float { return static_cast<float>(decrementor) / static_cast<float>(native_time); }
	[[nodiscard]] auto get_quadratic_normalized() const -> float { return static_cast<float>(decrementor * decrementor) / static_cast<float>(native_time * native_time); }
	[[nodiscard]] auto get_cubic_normalized() const -> float { return static_cast<float>(decrementor * decrementor * decrementor) / static_cast<float>(native_time * native_time * native_time); }
	[[nodiscard]] auto get_inverse_normalized() const -> float { return 1.f - get_normalized(); }
	[[nodiscard]] auto get_inverse_quadratic_normalized() const -> float { return 1.f - get_quadratic_normalized(); }
	[[nodiscard]] auto get_inverse_cubic_normalized() const -> float { return 1.f - get_cubic_normalized(); }
	[[nodiscard]] auto get_native_time() const -> int { return native_time; }
	[[nodiscard]] auto null() const -> bool { return decrementor == -1; }

  private:
	int decrementor{};
	int native_time{};
};

} // namespace fornani::util
