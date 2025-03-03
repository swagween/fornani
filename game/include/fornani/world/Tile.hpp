#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Shape.hpp"

namespace fornani::automa {
struct ServiceProvider;
}
namespace fornani::player {
class Player;
}
namespace fornani::arms {
class Projectile;
}
namespace fornani::world {
class Map;
enum class TileType : std::uint8_t { empty, solid, platform, ceiling_ramp, ground_ramp, spike, big_spike, breakable, pushable, target, spawner, checkpoint, bonfire, campfire };
enum class TileState : std::uint8_t { ramp_adjacent, big_ramp, covered };
constexpr static int special_index_v{448};

struct Tile {

	Tile() = default;
	constexpr static int evaluate(uint32_t val) {
		auto ret{4};
		if (val == special_index_v + 3 || val == special_index_v + 4 || val == special_index_v + 9 || val == special_index_v + 10 || val == special_index_v + 13 || val == special_index_v + 14) { ret = 3; }
		if (val == special_index_v + 19 || val == special_index_v + 20 || val == special_index_v + 25 || val == special_index_v + 26 || val == special_index_v + 29 || val == special_index_v + 30) { ret = 3; }
		if ((val >= special_index_v + 32 && val <= special_index_v + 35) || (val >= special_index_v + 48 && val <= special_index_v + 51)) { ret = 3; }
		return ret;
	}
	Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val, uint32_t odi, float spacing);

	void on_hit(automa::ServiceProvider& svc, player::Player& player, world::Map& map, arms::Projectile& proj);
	void render(sf::RenderWindow& win, sf::RectangleShape& draw, sf::Vector2<float> cam);
	void draw(sf::RenderTexture& tex);
	void set_type();
	[[nodiscard]] auto is_occupied() const -> bool { return value > 0; }
	[[nodiscard]] auto is_collidable() const -> bool { return type == TileType::solid || is_ramp() || is_spawner() || is_platform(); }
	[[nodiscard]] auto is_solid() const -> bool { return type == TileType::solid; }
	[[nodiscard]] auto is_hookable() const -> bool { return type == TileType::solid; }
	[[nodiscard]] auto is_ramp() const -> bool { return type == TileType::ground_ramp || type == TileType::ceiling_ramp; }
	[[nodiscard]] auto is_big_ramp() const -> bool { return flags.test(TileState::big_ramp); }
	[[nodiscard]] auto is_ground_ramp() const -> bool { return type == TileType::ground_ramp; }
	[[nodiscard]] auto is_ceiling_ramp() const -> bool { return type == TileType::ceiling_ramp; }
	[[nodiscard]] auto is_platform() const -> bool { return type == TileType::platform; }
	[[nodiscard]] auto is_spike() const -> bool { return type == TileType::spike; }
	[[nodiscard]] auto is_big_spike() const -> bool { return type == TileType::big_spike; }
	[[nodiscard]] auto is_breakable() const -> bool { return type == TileType::breakable; }
	[[nodiscard]] auto is_pushable() const -> bool { return type == TileType::pushable; }
	[[nodiscard]] auto is_spawner() const -> bool { return type == TileType::spawner; }
	[[nodiscard]] auto is_target() const -> bool { return type == TileType::target; }
	[[nodiscard]] auto is_checkpoint() const -> bool { return type == TileType::checkpoint; }
	[[nodiscard]] auto is_fire() const -> bool { return type == TileType::bonfire || type == TileType::campfire; }
	[[nodiscard]] auto is_special() const -> bool { return is_pushable() || is_breakable() || is_target() || is_checkpoint() || is_fire() || is_spike() || is_big_spike(); }
	[[nodiscard]] auto ramp_adjacent() const -> bool { return flags.test(TileState::ramp_adjacent); }
	[[nodiscard]] auto covered() const -> bool { return flags.test(TileState::covered); }
	[[nodiscard]] auto is_negative_ramp() const -> bool {
		return (value >= special_index_v + 16 && value < special_index_v + 20) || (value == special_index_v + 24 || value == special_index_v + 25) || (value == special_index_v + 28 || value == special_index_v + 29) ||
			   (value == special_index_v + 48 || value == special_index_v + 50);
	}
	[[nodiscard]] auto is_positive_ramp() const -> bool { return is_ground_ramp() && !is_negative_ramp(); }
	[[nodiscard]] auto scaled_position() const -> sf::Vector2<int> { return sf::Vector2<int>{static_cast<int>(bounding_box.get_position().x), static_cast<int>(bounding_box.get_position().y)}; }
	[[nodiscard]] auto f_scaled_position() const -> sf::Vector2f { return bounding_box.get_position() / m_spacing; }
	[[nodiscard]] auto get_center() const -> sf::Vector2<float> { return bounding_box.get_position() + bounding_box.get_dimensions() * 0.5f; }
	[[nodiscard]] auto position() const -> sf::Vector2<float> { return bounding_box.get_position(); }

	sf::Vector2<uint32_t> index;
	uint32_t one_d_index;

	uint32_t value;
	TileType type{};
	shape::Shape bounding_box;

	bool collision_check{};
	bool surrounded{};
	bool exposed{};
	mutable bool debug_flag{};
	util::BitFlags<TileState> flags{};

  private:
	float m_spacing;
};

} // namespace fornani::world
