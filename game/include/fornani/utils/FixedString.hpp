#pragma once

// If you have questions about this, ask Ian.

#include <algorithm>
#include <array>
#include <string_view>

namespace fornani
{
inline constexpr std::size_t fixed_string_capacity_v{64};

///
/// \brief Discount fixed capacity string.
///
template <std::size_t Capacity = fixed_string_capacity_v>
class FixedString
{
public:
	FixedString() = default;

	constexpr FixedString(std::string_view const text) : m_size(std::min(Capacity, text.size())) { std::copy_n(text.begin(), m_size, m_buffer.data()); }

	[[nodiscard]] constexpr std::string_view view() const { return std::string_view{m_buffer.data(), m_size}; }

	constexpr operator std::string_view() const { return view(); }

private:
	std::array<char, Capacity> m_buffer{};
	std::size_t m_size{};
};

// unit tests

static_assert(FixedString{"hello world"}.view() == "hello world");
} // namespace fornani
