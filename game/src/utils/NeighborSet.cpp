
#include <fornani/utils/NeighborSet.hpp>
#include <fornani/world/Tile.hpp>
#include <ranges>

namespace fornani {

auto NeighborSet::get_rotation_via(std::uint32_t check) const -> sf::Angle {
	for (auto const& n : m_cardinal_neighbors) {
		if (n == check) { return CardinalDirection(static_cast<UDLR>(n)).as_angle() + sf::degrees(180.f); }
	}
	return sf::degrees(0.f);
}

auto NeighborSet::get_direction_via(std::uint32_t check) const -> CardinalDirection {
	for (auto [i, n] : std::views::enumerate(m_cardinal_neighbors)) {
		if (n == check) { return CardinalDirection(static_cast<UDLR>(i)).opposite(); }
	}
	for (auto [i, n] : std::views::enumerate(m_cardinal_neighbors)) {
		if (n != 0 && n < world::special_index_v) { return CardinalDirection(static_cast<UDLR>(i)).opposite(); }
	}
	return CardinalDirection{};
}

} // namespace fornani
