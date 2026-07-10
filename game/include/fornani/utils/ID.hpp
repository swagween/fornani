
#pragma once

#include <compare>
#include <cstdint>
#include <functional>
#include <stdexcept>

namespace fornani {

enum class IDFamily { none, entity, utility, scene };

class ID {
  public:
	ID(int const specifier) : m_specifier{static_cast<std::uint32_t>(specifier)} {}
	ID(int const family, int const specifier, int const unique) : m_family{static_cast<std::uint32_t>(family)}, m_specifier{static_cast<std::uint32_t>(specifier)}, m_unique{static_cast<std::uint32_t>(unique)} {}

	[[nodiscard]] auto get_family() const -> int { return static_cast<int>(m_specifier); }
	[[nodiscard]] auto get_specifier() const -> int { return static_cast<int>(m_specifier); }
	[[nodiscard]] auto get_unique() const -> int { return static_cast<int>(m_unique); }
	[[nodiscard]] auto get() const -> int { return get_specifier(); }

  private:
	std::uint32_t m_family{};
	std::uint32_t m_specifier{};
	std::uint32_t m_unique{};
};

class StableID {
  public:
	using underlying_type = std::uint64_t;

	constexpr StableID() noexcept = default;

	constexpr explicit StableID(underlying_type value) noexcept : m_value(value) {}

	static constexpr StableID from(int code, int x, int y) noexcept {
		return StableID{(static_cast<std::uint64_t>(static_cast<std::uint32_t>(code)) << 32) | (static_cast<std::uint64_t>(static_cast<std::uint16_t>(x)) << 16) | static_cast<std::uint64_t>(static_cast<std::uint16_t>(y))};
	}

	constexpr underlying_type get() const noexcept { return m_value; }
	constexpr auto operator<=>(StableID const&) const noexcept = default;

  private:
	underlying_type m_value{0};
};

} // namespace fornani
