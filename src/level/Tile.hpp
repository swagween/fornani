#pragma once
#include <SFML/Graphics.hpp>
#include "../setup/EnumLookups.hpp"
#include "../utils/Shape.hpp"

namespace world {

enum class TileType { empty, solid, platform, ceiling_ramp, ground_ramp, spike, death_spike, breakable };

struct Tile {

	Tile() = default;
	Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val);

	void update_polygon(sf::Vector2<float> cam); // for greyblock mode
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_type();
	[[nodiscard]] auto is_occupied() const -> bool { return value > 0; }
	[[nodiscard]] auto is_collidable() const -> bool { return type == TileType::solid || type == TileType::ground_ramp || type == TileType::ceiling_ramp; }
	[[nodiscard]] auto is_solid() const -> bool { return type == TileType::solid; }
	[[nodiscard]] auto is_hookable() const -> bool { return type == TileType::solid; }
	[[nodiscard]] auto is_ramp() const -> bool { return type == TileType::ground_ramp || type == TileType::ceiling_ramp; }
	[[nodiscard]] auto is_spike() const -> bool { return type == TileType::spike; }
	[[nodiscard]] auto is_death_spike() const -> bool { return type == TileType::death_spike; }
	[[nodiscard]] auto is_breakable() const -> bool { return type == TileType::breakable; }

	sf::Vector2<float> middle_point();

	sf::Vector2<uint32_t> index{};
	sf::Vector2<float> position{};
	uint32_t one_d_index{};

	uint8_t value{};
	TileType type{};	 // for assigning attributes
	shape::Shape bounding_box{}; // for collision

	bool collision_check{};
	bool surrounded{};
	
	sf::ConvexShape polygon{};
	sf::RectangleShape drawbox{};
};
}