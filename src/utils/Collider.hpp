
#pragma once

#include "../components/PhysicsComponent.hpp"
#include "../setup/EnumLookups.hpp"
#include "BitFlags.hpp"
#include "Shape.hpp"

namespace shape {

float const default_dim = 24.0f;

float const default_jumpbox_height = 2.0f;
float const default_detector_width = 4.f;
float const default_detector_height = 18.f;

enum class State {
	just_collided,
	is_any_jump_collision,
	is_any_collision,
	just_landed,
	ceiling_collision,
	grounded,
	on_ramp,
};

enum class Collision {
	any_collision,
	has_left_collision,
	has_right_collision,
	has_top_collision,
	has_bottom_collision,
	ramp_collision
};

struct PhysicsStats {
	float GRAV{0.002f};
};

class Collider {

  public:
	Collider();
	Collider(sf::Vector2<float> dim, sf::Vector2<float> start_pos = {0, 0});

	void sync_components();
	void handle_map_collision(Shape const& cell, lookup::TILE_TYPE tile_type);
	void correct_x();
	void correct_y();
	void handle_platform_collision(Shape const& cell);
	void handle_spike_collision(Shape const& cell);
	void handle_collider_collision(Shape const& collider);
	void update();
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
	void reset();
	void reset_ground_flags();

	bool on_ramp();
	bool has_horizontal_collision() const;
	bool has_left_collision() const;
	bool has_right_collision() const;
	bool has_vertical_collision() const;

	Shape bounding_box{};
	Shape predictive_vertical{};
	Shape predictive_horizontal{};
	Shape predictive_combined{};
	Shape jumpbox{};
	Shape hurtbox{};

	PhysicsStats stats{};
	components::PhysicsComponent physics{};
	util::BitFlags<State> flags{};
	util::BitFlags<Collision> collision_flags{};

	float landed_threshold{6.0f};
	float detector_buffer{19.0f};

	sf::Vector2<float> dimensions{};
	sf::Vector2<float> sprite_offset{};

	bool spike_trigger{};

	sf::RectangleShape box{};
	std::vector<std::string> inst;
};

} // namespace shape
