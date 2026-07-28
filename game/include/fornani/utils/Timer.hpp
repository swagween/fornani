
#pragma once

#include <algorithm>

namespace fornani::util {

class Timer {
  public:
	Timer() = default;
	explicit constexpr Timer(float duration) : duration_(duration) {}

	constexpr void start() { remaining_ = duration_; }
	constexpr void start(float duration) {
		duration_ = duration;
		remaining_ = duration;
	}

	constexpr void update(float dt) { remaining_ = std::max(remaining_ - dt, 0.f); }
	constexpr void cancel() { remaining_ = 0.f; }
	[[nodiscard]] constexpr bool running() const { return remaining_ > 0.f; }
	[[nodiscard]] constexpr bool complete() const { return remaining_ <= 0.f; }
	[[nodiscard]] constexpr float remaining() const { return remaining_; }
	[[nodiscard]] constexpr float duration() const { return duration_; }
	[[nodiscard]] constexpr float normalized() const {
		if (duration_ <= 0.f) return 0.f;
		return remaining_ / duration_;
	}
	[[nodiscard]] constexpr float inverse_normalized() const { return 1.f - normalized(); }
	[[nodiscard]] constexpr float quadratic_normalized() const {
		auto t = normalized();
		return t * t;
	}
	[[nodiscard]] constexpr float cubic_normalized() const {
		auto t = normalized();
		return t * t * t;
	}
	[[nodiscard]] constexpr float quadratic_inverse_normalized() const {
		auto t = inverse_normalized();
		return t * t;
	}
	[[nodiscard]] constexpr float cubic_inverse_normalized() const {
		auto t = inverse_normalized();
		return t * t * t;
	}

  private:
	float remaining_{0.f};
	float duration_{0.f};
};

} // namespace fornani::util
