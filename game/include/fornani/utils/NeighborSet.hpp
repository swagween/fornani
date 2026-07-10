
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/Direction.hpp>
#include <array>
#include <cstdint>

namespace fornani {

struct NeighborSet {
	void set(UDLR dir, std::uint32_t value) { m_cardinal_neighbors.at(static_cast<std::size_t>(dir)) = value; }
	[[nodiscard]] auto get(UDLR dir) const -> std::uint32_t { return m_cardinal_neighbors.at(static_cast<std::size_t>(dir)); }
	[[nodiscard]] auto get_rotation_via(std::uint32_t check) const -> sf::Angle;
	[[nodiscard]] auto get_direction_via(std::uint32_t check) const -> CardinalDirection;

  private:
	std::array<std::uint32_t, 4> m_cardinal_neighbors{};
};

} // namespace fornani
