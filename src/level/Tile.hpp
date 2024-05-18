#pragma once
#include <SFML/Graphics.hpp>
#include "../setup/EnumLookups.hpp"
#include "../utils/Shape.hpp"

namespace world {

struct Tile {

	Tile() = default;
	Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val, lookup::TILE_TYPE t);

	void update_polygon(sf::Vector2<float> cam); // for greyblock mode
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);

	bool is_occupied() const;
	bool is_collidable() const;
	bool is_solid() const;
	bool is_hookable() const;
	bool is_breakable() const;
	bool is_ramp() const;
	sf::Vector2<float> middle_point();

	sf::Vector2<uint32_t> index{};
	sf::Vector2<float> position{};
	uint32_t one_d_index{};

	uint8_t value{};
	lookup::TILE_TYPE type{};	 // for assigning attributes
	shape::Shape bounding_box{}; // for collision

	bool collision_check{};
	bool surrounded{};
	
	sf::ConvexShape polygon{};
	sf::RectangleShape drawbox{};
};
}