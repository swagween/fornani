//
//  Collider.hpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include "Shape.hpp"
#include "../components/PhysicsComponent.hpp"
#include "../setup/EnumLookups.hpp"
#include "BitFlags.hpp"

namespace shape {

	const float default_dim = 24.0f;

	const float default_jumpbox_height = 2.0f;
	const float default_detector_width = 2.0f;
	const float default_detector_height = 23.f;
	const float default_detector_buffer = (default_dim - default_detector_height) / 2;

	enum class State {
		just_collided,
		is_colliding_with_level,
		has_left_collision,
		has_right_collision,
		is_any_jump_collision,
		is_any_collision,
		just_landed,
		ceiling_collision,
		grounded,
	};

	class Collider {
	public:

		Collider();
		Collider(sf::Vector2<float> dim, sf::Vector2<float> start_pos);

		void sync_components();
		void handle_map_collision(const Shape& cell, lookup::TILE_TYPE tile_type);
		void handle_platform_collision(const Shape& cell);
		void handle_spike_collision(const Shape& cell);
		void update();
		void render(sf::RenderWindow& win, sf::Vector2<float> cam);
		void reset();
		void reset_ground_flags();

		Shape bounding_box{};
		Shape predictive_bounding_box{};
		Shape jumpbox{};
		Shape left_detector{};
		Shape right_detector{};
		Shape hurtbox{};

		components::PhysicsComponent physics{};
		util::BitFlags<State> flags{};

		float landed_threshold{ 1.0f };

		//prob not needed or used...
		int left_aabb_counter{ 0 };
		int right_aabb_counter{ 0 };

		sf::Vector2<float>dimensions{};

		bool spike_trigger{};

		sf::RectangleShape box{};

	};

}

 /* Clock_hpp */
