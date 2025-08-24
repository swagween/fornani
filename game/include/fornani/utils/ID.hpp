
#pragma once

#include <cstdint>

namespace fornani {

enum class IDFamily : std::uint8_t { none, entity, utility, scene };

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

} // namespace fornani
