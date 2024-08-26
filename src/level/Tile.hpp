#pragma once
#include <SFML/Graphics.hpp>
#include "../setup/EnumLookups.hpp"
#include "../utils/Shape.hpp"
#include "../utils/BitFlags.hpp"

namespace world {

enum class TileType { empty, solid, platform, ceiling_ramp, ground_ramp, spike, death_spike, breakable, pushable, spawner };
enum class TileState { ramp_adjacent, big_ramp };

struct Tile {

	Tile() = default;
	Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val);

	void update_polygon(sf::Vector2<float> cam); // for greyblock mode
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_type();
	[[nodiscard]] auto is_occupied() const -> bool { return value > 0; }
	[[nodiscard]] auto is_collidable() const -> bool { return type == TileType::solid || is_ramp() || is_spawner(); }
	[[nodiscard]] auto is_solid() const -> bool { return type == TileType::solid; }
	[[nodiscard]] auto is_hookable() const -> bool { return type == TileType::solid; }
	[[nodiscard]] auto is_ramp() const -> bool { return type == TileType::ground_ramp || type == TileType::ceiling_ramp; }
	[[nodiscard]] auto is_big_ramp() const -> bool { return flags.test(TileState::big_ramp); }
	[[nodiscard]] auto is_ground_ramp() const -> bool { return type == TileType::ground_ramp; }
	[[nodiscard]] auto is_ceiling_ramp() const -> bool { return type == TileType::ceiling_ramp; }
	[[nodiscard]] auto is_platform() const -> bool { return type == TileType::platform; }
	[[nodiscard]] auto is_spike() const -> bool { return type == TileType::spike; }
	[[nodiscard]] auto is_death_spike() const -> bool { return type == TileType::death_spike; }
	[[nodiscard]] auto is_breakable() const -> bool { return type == TileType::breakable; }
	[[nodiscard]] auto is_pushable() const -> bool { return type == TileType::pushable; }
	[[nodiscard]] auto is_spawner() const -> bool { return type == TileType::spawner; }
	[[nodiscard]] auto is_special() const -> bool { return is_pushable() || is_breakable(); }
	[[nodiscard]] auto ramp_adjacent() const -> bool { return flags.test(TileState::ramp_adjacent); }

	sf::Vector2<float> middle_point();

	sf::Vector2<uint32_t> index{};
	sf::Vector2<float> position{};
	sf::Vector2<int> scaled_position{};
	uint32_t one_d_index{};

	uint8_t value{};
	TileType type{};	 // for assigning attributes
	shape::Shape bounding_box{}; // for collision

	bool collision_check{};
	bool surrounded{};
	util::BitFlags<TileState> flags{};
	
	sf::ConvexShape polygon{};
	sf::RectangleShape drawbox{};
};
}