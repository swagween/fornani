
#pragma once

#include <stdio.h>
#include <cmath>
#include <iostream>
#include <vector>
#include "../setup/EnumLookups.hpp"
#include "Tile.hpp"
#include <djson/json.hpp>

namespace world {

int const CEIL_SLANT_INDEX = 192;
int const FLOOR_SLANT_INDEX = 208;

class Grid {
  public:
	Grid() = default;
	Grid(sf::Vector2<uint32_t> d, dj::Json& source);

	sf::Vector2<uint32_t> dimensions{};
	std::vector<Tile> cells{};

	void check_neighbors(int i);
	void seed_vertex(int index);
	void destroy_cell(sf::Vector2<int> pos);

  private:
	float spacing{32.f};
};

} // namespace squid
