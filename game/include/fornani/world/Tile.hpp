
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/physics/Shape.hpp>

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

enum class TileType { empty, solid, platform, ceiling_ramp, ground_ramp, spike, big_spike, breakable, pushable, target, spawner, checkpoint, bonfire, campfire, home, incinerite };
enum class TileState { ramp_adjacent, big_ramp, covered };

constexpr static int special_index_v{448};

constexpr auto get_type_by_value(int const val) -> TileType {
	if (val < special_index_v && val > 0) { return TileType::solid; }
	if ((val < special_index_v + 16 && val >= special_index_v) || (val >= ceiling_single_ramp && val <= ceiling_single_ramp + 3)) { return TileType::ceiling_ramp; }
	if ((val < special_index_v + 32 && val >= special_index_v + 16) || (val >= floor_single_ramp && val <= floor_single_ramp + 3)) { return TileType::ground_ramp; }
	if (val < special_index_v + 48 && val >= special_index_v + 44) { return TileType::platform; }
	if (val < special_index_v + 38 && val >= special_index_v + 36) { return TileType::pushable; }
	if (val == special_index_v + 38) { return TileType::target; }
	if (val == special_index_v + 39) { return TileType::spawner; }
	if (val == special_index_v + 40) { return TileType::home; }
	if (val == special_index_v + 41) { return TileType::incinerite; }
	if (val == special_index_v + 52) { return TileType::bonfire; }
	if (val == special_index_v + 53) { return TileType::campfire; }
	if (val == special_index_v + 54) { return TileType::checkpoint; }
	if (val == special_index_v + 55) { return TileType::breakable; }
	if (val == special_index_v + 62) { return TileType::big_spike; }
	if (val == special_index_v + 63) { return TileType::spike; }
	return TileType::empty;
}

struct Tile {

	Tile() = default;
	constexpr static int evaluate(std::uint32_t val) {
		auto ret{4};
		if (val == special_index_v + 3 || val == special_index_v + 4 || val == special_index_v + 9 || val == special_index_v + 10 || val == special_index_v + 13 || val == special_index_v + 14) { ret = 3; }
		if (val == special_index_v + 19 || val == special_index_v + 20 || val == special_index_v + 25 || val == special_index_v + 26 || val == special_index_v + 29 || val == special_index_v + 30) { ret = 3; }
		if ((val >= special_index_v + 32 && val <= special_index_v + 35) || (val >= special_index_v + 48 && val <= special_index_v + 51)) { ret = 3; }
		return ret;
	}
	Tile(sf::Vector2<std::uint32_t> i, sf::Vector2f p, std::uint32_t val, std::uint32_t odi, float spacing, std::uint8_t chunk_id);

	void on_hit(automa::ServiceProvider& svc, player::Player& player, world::Map& map, arms::Projectile& proj);
	void render(sf::RenderWindow& win, sf::RectangleShape& draw, sf::Vector2f cam);
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
	[[nodiscard]] auto is_home() const -> bool { return type == TileType::home; }
	[[nodiscard]] auto is_incinerite() const -> bool { return type == TileType::incinerite; }
	[[nodiscard]] auto is_checkpoint() const -> bool { return type == TileType::checkpoint; }
	[[nodiscard]] auto is_fire() const -> bool { return type == TileType::bonfire || type == TileType::campfire; }
	[[nodiscard]] auto is_special() const -> bool { return is_pushable() || is_breakable() || is_incinerite() || is_target() || is_checkpoint() || is_fire() || is_spike() || is_big_spike() || is_home(); }
	[[nodiscard]] auto ramp_adjacent() const -> bool { return flags.test(TileState::ramp_adjacent); }
	[[nodiscard]] auto covered() const -> bool { return flags.test(TileState::covered); }
	[[nodiscard]] auto is_negative_ramp() const -> bool {
		return (value >= special_index_v + 16 && value < special_index_v + 20) || (value == special_index_v + 24 || value == special_index_v + 25) || (value == special_index_v + 28 || value == special_index_v + 29) ||
			   (value == special_index_v + 48 || value == special_index_v + 50);
	}
	[[nodiscard]] auto is_positive_ramp() const -> bool { return is_ground_ramp() && !is_negative_ramp(); }
	[[nodiscard]] auto is_minimap_drawable() const -> bool { return is_checkpoint() || is_home() || is_fire() || is_target() || is_breakable(); }

	[[nodiscard]] auto scaled_position() const -> sf::Vector2i { return sf::Vector2i{bounding_box.get_position()}; }
	[[nodiscard]] auto f_scaled_position() const -> sf::Vector2f { return bounding_box.get_position() / m_spacing; }
	[[nodiscard]] auto get_local_center() const -> sf::Vector2f { return bounding_box.get_dimensions() * 0.5f; }
	[[nodiscard]] auto get_global_center() const -> sf::Vector2f { return bounding_box.get_position() + bounding_box.get_dimensions() * 0.5f; }
	[[nodiscard]] auto position() const -> sf::Vector2f { return bounding_box.get_position(); }
	[[nodiscard]] auto get_chunk_id() const -> std::uint8_t { return m_chunk_id; };

	sf::Vector2<std::uint32_t> index;
	std::uint32_t one_d_index;

	std::uint32_t value;
	TileType type{};
	shape::Shape bounding_box;

	bool collision_check{};
	bool surrounded{};
	bool exposed{};
	mutable bool debug_flag{};
	util::BitFlags<TileState> flags{};

  private:
	float m_spacing;
	std::uint8_t m_chunk_id{};
};

} // namespace fornani::world
