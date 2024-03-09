
#pragma once

#include <stdio.h>
#include <cmath>
#include <iostream>
#include <vector>
#include "../setup/EnumLookups.hpp"
#include "../utils/Shape.hpp"

namespace squid {

float const DEFAULT_SPACING = 32.0;

int const CEIL_SLANT_INDEX = 192;
int const FLOOR_SLANT_INDEX = 208;

struct Tile {

	Tile() = default;
	Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val, lookup::TILE_TYPE t) : index(i), position(p), value(val), type(t) { bounding_box = shape::Shape(sf::Vector2<float>(DEFAULT_SPACING, DEFAULT_SPACING)); }

	bool is_occupied() const { return value > 0; }
	bool is_collidable() const { return type != lookup::TILE_TYPE::TILE_CEILING_RAMP && type != lookup::TILE_TYPE::TILE_GROUND_RAMP && type != lookup::TILE_TYPE::TILE_SPIKES; }
	sf::Vector2<float> middle_point() { return {bounding_box.position.x + bounding_box.dimensions.x / 2, bounding_box.position.y + bounding_box.dimensions.y / 2}; }

	sf::Vector2<uint32_t> index{};
	sf::Vector2<float> position{};
	uint32_t one_d_index{};

	uint32_t value{};
	lookup::TILE_TYPE type{};	 // for assigning attributes
	shape::Shape bounding_box{}; // for collision

	bool collision_check{};
	bool surrounded{};
};

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
