
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Direction.hpp>

namespace fornani {

struct NeighborSet {
	util::BitFlags<UDLR> neighbors{};

	[[nodiscard]] auto get_rotation() const -> sf::Angle {
		auto ret = sf::Angle{};
		if (get_count() == 1) {
			if (neighbors.test(UDLR::down)) { ret = sf::degrees(0); }
			if (neighbors.test(UDLR::left)) { ret = sf::degrees(90); }
			if (neighbors.test(UDLR::up)) { ret = sf::degrees(180); }
			if (neighbors.test(UDLR::right)) { ret = sf::degrees(270); }
		}
		if (get_count() == 2) {
			if (as_direction().up()) { ret = sf::degrees(0); }
			if (as_direction().right()) { ret = sf::degrees(90); }
			if (as_direction().down()) { ret = sf::degrees(180); }
			if (as_direction().left()) { ret = sf::degrees(270); }
		}
		if (get_count() == 3) {
			if (neighbors.test(UDLR::down) && neighbors.test(UDLR::left) && neighbors.test(UDLR::right)) { ret = sf::degrees(0); }
			if (neighbors.test(UDLR::up) && neighbors.test(UDLR::left) && neighbors.test(UDLR::down)) { ret = sf::degrees(90); }
			if (neighbors.test(UDLR::up) && neighbors.test(UDLR::right) && neighbors.test(UDLR::left)) { ret = sf::degrees(180); }
			if (neighbors.test(UDLR::down) && neighbors.test(UDLR::right) && neighbors.test(UDLR::up)) { ret = sf::degrees(270); }
		}
		return ret;
	}

	[[nodiscard]] auto get_count() const -> int { return neighbors.count(); }
	[[nodiscard]] auto as_direction() const -> CardinalDirection {
		auto ret = CardinalDirection(UDLR::up);
		if (get_count() == 1) {
			if (neighbors.test(UDLR::up)) { ret = CardinalDirection(UDLR::down); }
			if (neighbors.test(UDLR::down)) { ret = CardinalDirection(UDLR::up); }
			if (neighbors.test(UDLR::left)) { ret = CardinalDirection(UDLR::right); }
			if (neighbors.test(UDLR::right)) { ret = CardinalDirection(UDLR::left); }
		}
		if (get_count() == 2) {
			if (neighbors.test(UDLR::down) && neighbors.test(UDLR::left)) { ret = CardinalDirection(UDLR::up); }
			if (neighbors.test(UDLR::up) && neighbors.test(UDLR::left)) { ret = CardinalDirection(UDLR::right); }
			if (neighbors.test(UDLR::up) && neighbors.test(UDLR::right)) { ret = CardinalDirection(UDLR::down); }
			if (neighbors.test(UDLR::down) && neighbors.test(UDLR::right)) { ret = CardinalDirection(UDLR::left); }
		}
		if (get_count() == 3) {
			if (!neighbors.test(UDLR::up)) { ret = CardinalDirection(UDLR::up); }
			if (!neighbors.test(UDLR::down)) { ret = CardinalDirection(UDLR::down); }
			if (!neighbors.test(UDLR::left)) { ret = CardinalDirection(UDLR::left); }
			if (!neighbors.test(UDLR::right)) { ret = CardinalDirection(UDLR::right); }
		}
		return ret;
	}
};

} // namespace fornani
