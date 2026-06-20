
#pragma once

#include <algorithm>
#include <limits>

namespace fornani::util {

class Counter {
  public:
	constexpr void start() { incrementor = 0; }
	constexpr void update(int const amount = 1) { incrementor = std::clamp(incrementor + amount, 0, std::numeric_limits<int>::max()); }
	constexpr void set(int const value) { incrementor = value; }
	constexpr void cancel() { incrementor = -1; }
	[[nodiscard]] auto running() const -> bool { return incrementor != 0; }
	[[nodiscard]] auto canceled() const -> bool { return incrementor == -1; }
	[[nodiscard]] auto get_count() const -> int { return incrementor; }
	template <typename T>
	T as() const {
		return static_cast<T>(incrementor);
	}

  private:
	int incrementor{};
};

constexpr auto default_interval{0.1f};
class FloatCounter {
  public:
	FloatCounter(float const interval = default_interval) : m_interval{interval} {}
	constexpr void reset() { m_value = 0.f; }
	constexpr void set_cycle_point(float const to) { m_cycle_point = to; }
	constexpr void update(float amount) noexcept {
		m_value += amount;
		if (m_value >= m_cycle_point) { m_value -= m_cycle_point; }
	}
	constexpr void update() noexcept { update(m_interval); }
	constexpr void set(int const value) { m_value = value; }
	constexpr void cancel() { m_value = -1.f; }
	[[nodiscard]] auto running() const -> bool { return m_value != 0.f; }
	[[nodiscard]] auto canceled() const -> bool { return m_value == -1.f; }
	[[nodiscard]] auto get() const -> float { return m_value; }
	template <typename T>
	T as() const {
		return static_cast<T>(get());
	}

  private:
	float m_value{};
	float m_interval{};
	float m_cycle_point{std::numeric_limits<float>::max()};
};

} // namespace fornani::util
