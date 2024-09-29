#pragma once
#include <SFML/Graphics.hpp>
#include "../setup/EnumLookups.hpp"
#include "../utils/Shape.hpp"
#include "../utils/BitFlags.hpp"

namespace automa {
struct ServiceProvider;
}
namespace player {
class Player;
}
namespace arms {
class Projectile;
}
namespace world {
class Map;
enum class TileType { empty, solid, platform, ceiling_ramp, ground_ramp, spike, death_spike, breakable, pushable, spawner };
enum class TileState { ramp_adjacent, big_ramp };

struct Tile {

	Tile() = default;
	Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val, uint32_t odi);

	void on_hit(automa::ServiceProvider& svc, player::Player& player, world::Map& map, arms::Projectile& proj);
	void update_polygon(sf::Vector2<float> cam); // for greyblock mode
	void render(sf::RenderWindow& win, sf::Vector2<float> cam, sf::RectangleShape& draw); // greyblock mode only
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
	[[nodiscard]] auto is_negative_ramp() const -> bool { return (value >= 208 && value < 212) || (value == 216 || value == 217) || (value == 220 || value == 221); }
	[[nodiscard]] auto is_positive_ramp() const -> bool { return is_ground_ramp() && !is_negative_ramp(); }
	[[nodiscard]] auto scaled_position() const -> sf::Vector2<int> { return sf::Vector2<int>{static_cast<int>(bounding_box.position.x), static_cast<int>(bounding_box.position.y)}; }
	[[nodiscard]] auto middle_point() const -> sf::Vector2<float> { return bounding_box.position + bounding_box.dimensions * 0.5f; }
	[[nodiscard]] auto position() const -> sf::Vector2<float> { return bounding_box.position; }

	sf::Vector2<uint32_t> index{};
	uint32_t one_d_index{};

	uint8_t value{};
	TileType type{};					 // for assigning attributes
	shape::Shape bounding_box{{32, 32}}; // for collision

	bool collision_check{};
	bool surrounded{};
	util::BitFlags<TileState> flags{};
};

} // namespace world