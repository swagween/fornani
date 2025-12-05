
#pragma once

#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/CollisionDepth.hpp>
#include <fornani/utils/Shape.hpp>
#include <fornani/world/Tile.hpp>
#include <optional>

namespace fornani::world {
class Map;
}

namespace fornani::shape {

constexpr auto default_dim = 24.0f;

constexpr auto default_jumpbox_height = 4.0f;
constexpr auto default_detector_width = 4.f;
constexpr auto default_detector_height = 18.f;

enum class General { ignore_resolution, complex, pushable, soft, top_only_collision, no_move };
enum class Animation { just_landed, sliding };
enum class State {
	just_collided,
	is_any_jump_collision,
	is_any_collision,
	just_landed,
	ceiling_collision,
	grounded,
	world_grounded,
	on_ramp,
	ledge_left,
	ledge_right,
	left_wallslide_collision,
	right_wallslide_collision,
	on_flat_surface,
	tickwise_ramp_collision
};
enum class ExternalState {
	grounded,
	collider_collision,
	vert_collider_collision,
	horiz_collider_collision,
	world_collision,
	horiz_world_collision,
	vert_world_collision,
	world_grounded,
	jumped_into,
	on_ramp,
	tile_debug_flag,
	ceiling_ramp_hit
};
enum class PermaFlags { world_grounded, downhill };

enum class Collision { any_collision, has_left_collision, has_right_collision, has_top_collision, has_bottom_collision, ramp_collision };
enum class Dash { dash_cancel_collision };
enum class Movement { dashing, jumping, crouching };

struct PhysicsStats {
	float GRAV{0.002f};
};

class Collider {

  public:
	Collider();
	explicit Collider(sf::Vector2f dim, sf::Vector2f hbx_offset = {});

	void sync_components();
	void handle_map_collision(world::Tile const& tile);
	void detect_map_collision(world::Map& map);
	void correct_x(sf::Vector2f mtv, bool has_velocity = false);
	void correct_y(sf::Vector2f mtv);
	void correct_x_y(sf::Vector2f mtv);
	void correct_corner(sf::Vector2f mtv);
	void resolve_depths();
	bool handle_collider_collision(Shape const& collider, bool soft = false, sf::Vector2f velocity = {}); // returns true if grounded on collider
	void handle_collider_collision(Collider const& collider, bool soft = false, bool momentum = false);
	void update(automa::ServiceProvider& svc, bool simple = false);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f pos);
	void set_direction(Direction to) { m_direction = to; }
	void reset();
	void reset_ground_flags();
	void set_top_only();
	void adjust_acceleration();
	void fix();

	bool on_ramp() const;
	bool has_horizontal_collision() const;
	bool has_left_collision() const;
	bool has_right_collision() const;
	bool has_vertical_collision() const;
	bool has_left_wallslide_collision() const;
	bool has_right_wallslide_collision() const;
	bool has_jump_collision() const;
	bool horizontal_squish() const;
	bool vertical_squish() const;
	bool pushes(Collider& other) const;

	sf::Vector2f snap_to_grid(float size = 1.f, float scale = 32.f, float factor = 2.f);
	[[nodiscard]] auto grounded() const -> bool { return flags.external_state.test(ExternalState::grounded); }
	[[nodiscard]] auto jumping() const -> bool { return flags.movement.test(Movement::jumping); }
	[[nodiscard]] auto world_grounded() const -> bool { return flags.state.test(State::world_grounded); }
	[[nodiscard]] auto external_world_grounded() const -> bool { return flags.external_state.test(ExternalState::world_grounded); }
	[[nodiscard]] auto jumped_into() -> bool { return flags.external_state.consume(ExternalState::jumped_into); }
	[[nodiscard]] auto perma_grounded() const -> bool { return flags.perma_state.test(PermaFlags::world_grounded); }
	[[nodiscard]] auto crushed() const -> bool { return collision_depths ? collision_depths.value().crushed() : false; }
	[[nodiscard]] auto get_center() const -> sf::Vector2f { return physics.position + dimensions * 0.5f; }
	[[nodiscard]] auto get_top() const -> sf::Vector2f { return sf::Vector2f{physics.position.x + dimensions.x * 0.5f, physics.position.y}; }
	[[nodiscard]] auto get_bottom() const -> sf::Vector2f { return sf::Vector2f{physics.position.x + dimensions.x * 0.5f, physics.position.y + dimensions.y}; }
	[[nodiscard]] auto get_average_tick_position() const -> sf::Vector2f { return physics.previous_position; }
	[[nodiscard]] auto get_below_point(int side = 0) const -> sf::Vector2f {
		return side == 0 ? jumpbox.get_position() + jumpbox.get_dimensions() * 0.5f : side == -1 ? jumpbox.get_position() + sf::Vector2f{0.f, 4.f} : jumpbox.get_position() + jumpbox.get_dimensions() - sf::Vector2f{0.f, 4.f};
	}
	[[nodiscard]] auto platform_collision() const -> bool { return flags.external_state.test(ExternalState::collider_collision); }
	[[nodiscard]] auto left() const -> float { return bounding_box.left(); }
	[[nodiscard]] auto right() const -> float { return bounding_box.right(); }
	[[nodiscard]] auto top() const -> float { return bounding_box.top(); }
	[[nodiscard]] auto bottom() const -> float { return bounding_box.bottom(); }
	[[nodiscard]] auto downhill() const -> bool { return flags.perma_state.test(PermaFlags::downhill); }
	[[nodiscard]] auto hit_ceiling_ramp() const -> bool { return flags.external_state.test(ExternalState::ceiling_ramp_hit); }
	[[nodiscard]] auto get_direction() const -> Direction { return m_direction; }

	Shape bounding_box{};
	Shape predictive_vertical{};
	Shape predictive_horizontal{};
	Shape predictive_combined{};
	Shape vicinity{};
	Shape wallslider{};
	Shape jumpbox{};
	Shape hurtbox{};
	Shape horizontal{};
	Shape vertical{};

	PhysicsStats stats{};
	components::PhysicsComponent physics{};

	std::optional<util::CollisionDepth> collision_depths{};

	struct {
		util::BitFlags<General> general{};
		util::BitFlags<State> state{};
		util::BitFlags<ExternalState> external_state{};
		util::BitFlags<PermaFlags> perma_state{};
		util::BitFlags<Animation> animation{};
		util::BitFlags<Collision> collision{};
		util::BitFlags<Movement> movement{};
		util::BitFlags<Dash> dash{};
	} flags{};

	struct {
		sf::Vector2f combined{};
		sf::Vector2f horizontal{};
		sf::Vector2f vertical{};
		sf::Vector2f actual{};
	} mtvs{};

	struct {
		sf::Color local{};
	} colors{};

	float vert_threshold{0.6f}; // for landing
	float horizontal_detector_buffer{1.0f};
	float vertical_detector_buffer{1.0f};
	float depth_buffer{1.0f};
	float wallslide_pad{5.f};

	float acceleration_multiplier{1.f};

	sf::Vector2f dimensions{};
	sf::Vector2f sprite_offset{};
	sf::Vector2f hurtbox_offset{};
	float maximum_ramp_height{};

	sf::RectangleShape box{};
	sf::RectangleShape draw_hurtbox{};

	Direction m_direction{};

	io::Logger m_logger{"Collider"};
};

} // namespace fornani::shape
