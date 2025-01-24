
#pragma once

#include "editor/canvas/Tile.hpp"
#include <stdio.h>
#include <vector>
#include <cmath>

namespace pi {

class Grid {
  public:
	Grid(sf::Vector2<uint32_t> d);
	void set_position(sf::Vector2<float> to_position, float scale);
	void match(Grid& other);
	sf::Vector2<uint32_t> dimensions{};
	std::vector<Tile> cells{};
};

} // namespace pi
