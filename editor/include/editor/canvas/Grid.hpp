
#pragma once

#include <vector>
#include "editor/canvas/Tile.hpp"

namespace pi {

class Grid {
  public:
	Grid(sf::Vector2<uint32_t> d);
	void set_position(sf::Vector2<float> to_position, float scale);
	void match(Grid& other);
	sf::Vector2<uint32_t> first_available_ground() const;
	sf::Vector2<uint32_t> dimensions{};
	std::vector<Tile> cells{};
};

} // namespace pi
