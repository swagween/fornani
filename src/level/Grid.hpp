
#pragma once

#include <stdio.h>
#include <cmath>
#include <iostream>
#include <vector>
#include "../setup/EnumLookups.hpp"
#include "Tile.hpp"

namespace world {

int const CEIL_SLANT_INDEX = 192;
int const FLOOR_SLANT_INDEX = 208;

class Grid {
  public:
	Grid() = default;
	Grid(sf::Vector2<uint32_t> d);

	sf::Vector2<uint32_t> dimensions{};
	std::vector<Tile> cells{};

	void update();
	void check_neighbors();

  private:
	void init_shape_vertices();
	void push_cells(int i);
	float spacing{};
};

} // namespace squid
