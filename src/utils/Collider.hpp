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

namespace shape {

	const float default_dim = 24.0f;

	const float default_jumpbox_height = 2.0f;
	const float default_detector_width = 4.0f;
	const float default_detector_height = 23.0f;
	const float default_detector_buffer = (default_dim - default_detector_height) / 2;

	class Collider {
	public:

		Collider();
		Collider(sf::Vector2<float> dim, sf::Vector2<float> start_pos);

		void sync_components();
		void handle_map_collision(const Shape& cell, lookup::TILE_TYPE tile_type);

		Shape bounding_box{};
		Shape predictive_bounding_box{};
		Shape jumpbox{};
		Shape left_detector{};
		Shape right_detector{};

		components::PhysicsComponent physics{};

		bool just_collided{ false };
		bool is_colliding_with_level{};
		bool has_left_collision{};
		bool has_right_collision{};
		bool is_any_jump_colllision{ false };
		bool is_any_colllision{ false };
		bool just_landed{};
		bool ceiling_collision{ false };

		float landed_threshold{ 1.0f };

		//prob not needed or used...
		int left_aabb_counter{ 0 };
		int right_aabb_counter{ 0 };

		sf::Vector2<float>dimensions{};

		bool spike_trigger{};

	};

}

 /* Clock_hpp */
