
#pragma once

#include "../components/PhysicsComponent.hpp"
#include "../level/Tile.hpp"
#include "BitFlags.hpp"
#include "Shape.hpp"

namespace world{
class Map;
}

namespace shape {

float const default_dim = 24.0f;
float const vicinity_pad = 16.f;
float const wallslide_pad = 2.f;

float const default_jumpbox_height = 4.0f;
float const default_detector_width = 4.f;
float const default_detector_height = 18.f;

enum class General { ignore_resolution, complex };
enum class Animation { just_landed };
enum class State { just_collided, is_any_jump_collision, is_any_collision, just_landed, ceiling_collision, grounded, world_grounded, on_ramp, ledge_left, ledge_right, left_wallslide_collision, right_wallslide_collision };
enum class ExternalState { grounded, collider_collision, vert_collider_collision, horiz_collider_collision, world_collision, horiz_world_collision, vert_world_collision };

enum class Collision {
	any_collision,
	has_left_collision,
	has_right_collision,
	has_top_collision,
	has_bottom_collision,
	ramp_collision
};
enum class Dash { dash_cancel_collision };
enum class Movement { dashing, jumping };

struct PhysicsStats {
	float GRAV{0.002f};
};

class Collider {

  public:
	Collider();
	Collider(sf::Vector2<float> dim, sf::Vector2<float> start_pos = {});

	void sync_components();
	void handle_map_collision(world::Tile const& tile);
	void detect_map_collision(world::Map& map);
	int detect_ledge_height(world::Map& map);
	void correct_x(sf::Vector2<float> mtv);
	void correct_y(sf::Vector2<float> mtv);
	void correct_x_y(sf::Vector2<float> mtv);
	void correct_corner(sf::Vector2<float> mtv);
	void resolve_depths();
	void set_depths();
	void handle_platform_collision(Shape const& cell);
	void handle_spike_collision(Shape const& cell);
	void handle_collider_collision(Shape const& collider);
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
	void reset();
	void reset_ground_flags();

	bool on_ramp();
	bool has_horizontal_collision() const;
	bool has_left_collision() const;
	bool has_right_collision() const;
	bool has_vertical_collision() const;
	bool has_left_wallslide_collision() const;
	bool has_right_wallslide_collision() const;
	bool horizontal_squish() const;
	bool vertical_squish() const;

	void flush_positions() { position_history.clear(); }
	sf::Vector2<float> get_average_tick_position();

	[[nodiscard]] auto grounded() const -> bool { return flags.external_state.test(ExternalState::grounded); }
	[[nodiscard]] auto world_grounded() const -> bool { return flags.state.test(State::world_grounded); }
	[[nodiscard]] auto crushed() const -> bool { return (collision_depths.top > crush_threshold && collision_depths.bottom > crush_threshold) || (collision_depths.left > crush_threshold && collision_depths.right > crush_threshold); }
	[[nodiscard]] auto get_center() const -> sf::Vector2<float> { return physics.position + dimensions * 0.5f; }
	[[nodiscard]] auto platform_collision() const -> bool { return flags.external_state.test(ExternalState::collider_collision); }
	[[nodiscard]] auto left() const -> float { return bounding_box.left(); }
	[[nodiscard]] auto right() const -> float { return bounding_box.right(); }
	[[nodiscard]] auto top() const -> float { return bounding_box.top(); }
	[[nodiscard]] auto bottom() const -> float { return bounding_box.bottom(); }
	
	float compute_length(sf::Vector2<float> const v);

	Shape bounding_box{};
	Shape predictive_vertical{};
	Shape predictive_horizontal{};
	Shape predictive_combined{};
	Shape vicinity{};
	Shape wallslider{};
	Shape jumpbox{};
	Shape hurtbox{};

	PhysicsStats stats{};
	components::PhysicsComponent physics{};

	struct {
		util::BitFlags<General> general{};
		util::BitFlags<State> state{};
		util::BitFlags<ExternalState> external_state{};
		util::BitFlags<Animation> animation{};
		util::BitFlags<Collision> collision{};
		util::BitFlags<Movement> movement{};
		util::BitFlags<Dash> dash{};
	} flags{};

	struct {
		sf::Vector2<float> combined{};
		sf::Vector2<float> horizontal{};
		sf::Vector2<float> vertical{};
		sf::Vector2<float> actual{};
	} mtvs{};

	struct {
		float top{};
		float bottom{};
		float left{};
		float right{};
	} collision_depths{};

	float crush_threshold{2.0f};
	float depth_throwaway{8.0f};
	float landed_threshold{6.0f};
	float horizontal_detector_buffer{1.0f};
	float vertical_detector_buffer{2.0f};

	sf::Vector2<float> dimensions{};
	sf::Vector2<float> sprite_offset{};
	std::deque<sf::Vector2<float>> position_history{};


	bool spike_trigger{};

	sf::RectangleShape box{};
	sf::RectangleShape draw_hurtbox{};
};

} // namespace shape
