
#pragma once

#include <stdio.h>
#include <cmath>
#include <vector>
#include "../setup/EnumLookups.hpp"
#include "../utils/Shape.hpp"

namespace squid {

float const DEFAULT_SPACING = 32.0;

int const CEIL_SLANT_INDEX = 192;
int const FLOOR_SLANT_INDEX = 208;

struct Tile {

	Tile(){};
	Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val, lookup::TILE_TYPE t) : index(i), position(p), value(val), type(t) { bounding_box = shape::Shape(sf::Vector2<float>(DEFAULT_SPACING, DEFAULT_SPACING)); }

	sf::Vector2<uint32_t> index{};
	sf::Vector2<float> position{};

	uint32_t value{};
	lookup::TILE_TYPE type{};	 // for assigning attributes
	shape::Shape bounding_box{}; // for collision

	bool collision_check{};
};

class Grid {
  public:
	Grid();
	Grid(sf::Vector2<uint32_t> d);

	sf::Vector2<uint32_t> dimensions{};
	float spacing{};

	std::vector<Tile> cells;

	void initialize();
	void update();

	void init_shape_vertices();

	void set_spacing(float spc);

	void push_cells(int i);
};

} // namespace squid
