//
//  Collider.cpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#include "Collider.hpp"

namespace shape {

    Collider::Collider() {

        physics = components::PhysicsComponent({ 0.9, 0.9 }, 1);

        dimensions = sf::Vector2<float>{ default_dim, default_dim };


        bounding_box.vertices[0] = sf::Vector2<float>(0, 0);
        bounding_box.vertices[1] = sf::Vector2<float>(0 + default_dim, 0);
        bounding_box.vertices[2] = sf::Vector2<float>(0 + default_dim, 0 + default_dim);
        bounding_box.vertices[3] = sf::Vector2<float>(0, 0 + default_dim);

        predictive_bounding_box.vertices[0] = sf::Vector2<float>(0, 0);
        predictive_bounding_box.vertices[1] = sf::Vector2<float>(0 + default_dim, 0);
        predictive_bounding_box.vertices[2] = sf::Vector2<float>(0 + default_dim, 0 + default_dim);
        predictive_bounding_box.vertices[3] = sf::Vector2<float>(0, 0 + default_dim);

        jumpbox.vertices[0] = sf::Vector2<float>(0, 0 + default_dim - default_jumpbox_height);
        jumpbox.vertices[1] = sf::Vector2<float>(0 + default_dim, 0 + default_dim - default_jumpbox_height);
        jumpbox.vertices[2] = sf::Vector2<float>(0 + default_dim, 0 + default_dim + default_jumpbox_height);
        jumpbox.vertices[3] = sf::Vector2<float>(0, 0 + default_dim + default_jumpbox_height);


        left_detector.left_offset = default_detector_width - 0.01f;
        right_detector.right_offset = default_detector_width - 0.01f;;

        left_detector.vertices[0] = sf::Vector2<float>(0 - default_detector_width, 0 + default_detector_buffer);
        left_detector.vertices[1] = sf::Vector2<float>(0, 0 + default_detector_buffer);
        left_detector.vertices[2] = sf::Vector2<float>(0, 0 + default_detector_height);
        left_detector.vertices[3] = sf::Vector2<float>(0 + left_detector.left_offset - default_detector_width, 0 + default_detector_height);

        right_detector.vertices[0] = sf::Vector2<float>(0 + default_dim, 0 + default_detector_buffer);
        right_detector.vertices[1] = sf::Vector2<float>(0 + default_dim + default_detector_width, 0 + default_detector_buffer);
        right_detector.vertices[2] = sf::Vector2<float>(0 + default_dim + default_detector_width - right_detector.right_offset, 0 + default_detector_height);
        right_detector.vertices[3] = sf::Vector2<float>(0 + default_dim, 0 + default_detector_height);

    }

    Collider::Collider(sf::Vector2<float> dim, sf::Vector2<float> start_pos) : dimensions(dim) {

        physics = components::PhysicsComponent({ 0.9, 0.9 }, 1);

        bounding_box.vertices[0] = sf::Vector2<float>(start_pos.x, start_pos.y);
        bounding_box.vertices[1] = sf::Vector2<float>(start_pos.x + dim.x, start_pos.y);
        bounding_box.vertices[2] = sf::Vector2<float>(start_pos.x + dim.x, start_pos.y + dim.y);
        bounding_box.vertices[3] = sf::Vector2<float>(start_pos.x, start_pos.y + dim.y);

        predictive_bounding_box.vertices[0] = sf::Vector2<float>(start_pos.x, start_pos.y);
        predictive_bounding_box.vertices[1] = sf::Vector2<float>(start_pos.x + dim.x, start_pos.y);
        predictive_bounding_box.vertices[2] = sf::Vector2<float>(start_pos.x + dim.x, start_pos.y + dim.y);
        predictive_bounding_box.vertices[3] = sf::Vector2<float>(start_pos.x, start_pos.y + dim.y);

        jumpbox.vertices[0] = sf::Vector2<float>(start_pos.x, start_pos.y + dim.y - default_jumpbox_height);
        jumpbox.vertices[1] = sf::Vector2<float>(start_pos.x + dim.x, start_pos.y + dim.y - default_jumpbox_height);
        jumpbox.vertices[2] = sf::Vector2<float>(start_pos.x + dim.x, start_pos.y + dim.y + default_jumpbox_height);
        jumpbox.vertices[3] = sf::Vector2<float>(start_pos.x, start_pos.y + dim.y + default_jumpbox_height);


        left_detector.left_offset = 0.0f;
        right_detector.right_offset = 0.0f;

        left_detector.vertices[0] = sf::Vector2<float>(start_pos.x - default_detector_width, start_pos.y + default_detector_buffer);
        left_detector.vertices[1] = sf::Vector2<float>(start_pos.x, start_pos.y + default_detector_buffer);
        left_detector.vertices[2] = sf::Vector2<float>(start_pos.x, start_pos.y + default_detector_height);
        left_detector.vertices[3] = sf::Vector2<float>(start_pos.x + left_detector.left_offset - default_detector_width, start_pos.y + default_detector_height);

        right_detector.vertices[0] = sf::Vector2<float>(start_pos.x + dim.x, start_pos.y + default_detector_buffer);
        right_detector.vertices[1] = sf::Vector2<float>(start_pos.x + dim.x + default_detector_width, start_pos.y + default_detector_buffer);
        right_detector.vertices[2] = sf::Vector2<float>(start_pos.x + dim.x + default_detector_width - right_detector.right_offset, start_pos.y + default_detector_height);
        right_detector.vertices[3] = sf::Vector2<float>(start_pos.x + dim.x, start_pos.y + default_detector_height);


    }

    void Collider::sync_components() {
        bounding_box.update(physics.position.x, physics.position.y, dimensions.x , dimensions.y);
        predictive_bounding_box.update(physics.position.x + physics.velocity.x, physics.position.y + physics.velocity.y, dimensions.x, dimensions.y);
        jumpbox.update(physics.position.x, physics.position.y + dimensions.x, dimensions.x, default_jumpbox_height);
        left_detector.update(physics.position.x - default_detector_width, physics.position.y + default_detector_buffer, default_detector_width, default_detector_height);
        right_detector.update(physics.position.x + dimensions.x, physics.position.y + default_detector_buffer, default_detector_width, default_detector_height);
    }

    void Collider::handle_map_collision(const Shape& cell, lookup::TILE_TYPE tile_type) {

        //tile flags
        bool is_ramp = tile_type == lookup::TILE_TYPE::TILE_RAMP;
        bool is_plat = tile_type == lookup::TILE_TYPE::TILE_PLATFORM;
        bool is_spike = tile_type == lookup::TILE_TYPE::TILE_SPIKES;

        if (predictive_bounding_box.SAT(cell)) {
            if (!is_spike) {
                if (!is_plat) {
                    is_any_colllision = true;
                }
                //set mtv
                physics.mtv = predictive_bounding_box.testCollisionGetMTV(predictive_bounding_box, cell);

                if(is_plat) {
                    if (physics.mtv.x > 0.0f) { physics.mtv.x = 0.0f; }
                }

                if (is_ramp && abs(physics.velocity.y) < 0.0001f) {
                    physics.position.y += physics.mtv.y;
                }

                if (physics.velocity.y > 3.0f) {
                    physics.mtv.x = 0.0f;
                }
                //here, we can do MTV again with the player's predicted position based on velocity
                if (physics.velocity.y > -0.01f && predictive_bounding_box.shape_y < cell.shape_y) {
                    if (physics.velocity.y > landed_threshold) {
                        just_landed = true;
                    }
                    float ydist = predictive_bounding_box.shape_y - physics.position.y;
                    float correction = ydist + physics.mtv.y;
                    physics.position.y += correction;
                    physics.velocity.y = 0.0f;
                    physics.acceleration.y = 0.0f;
                }
                //player hits the ceiling
                if (physics.velocity.y < -0.01f && abs(physics.mtv.y) > 0.001f && !is_plat) {
                    float ydist = physics.position.y - predictive_bounding_box.shape_y;
                    float correction = ydist + physics.mtv.y;
                    physics.position.y += correction;
                    physics.acceleration.y = 0.0f;
                    physics.velocity.y *= -0.5;
                    ceiling_collision = true;
                }

                //only for landing
                if (physics.velocity.y > 0.0f && !has_left_collision && !has_right_collision && !is_plat) {
                    physics.acceleration.y = 0.0f;
                    physics.velocity.y = 0.0f;
                }

            } else {
                spike_trigger = true;
            }

            physics.mtv = { 0.0f, 0.0f };
            just_collided = true;
            is_colliding_with_level = true;


        }

        float y_dist = cell.vertices[0].y - left_detector.vertices[2].y;
        sf::Vector2<float> detector_mtv = left_detector.testCollisionGetMTV(left_detector, cell);
		if (left_detector.SAT(cell) && !is_plat && !is_spike && !is_ramp) {
			if (!ceiling_collision && !just_landed) {
                has_left_collision = true;
				physics.acceleration.x = 0.0f;
				physics.velocity.x = 0.0f;
				physics.position.x += detector_mtv.x;
			}
			left_aabb_counter++;
		}
		detector_mtv = right_detector.testCollisionGetMTV(right_detector, cell);
		if (right_detector.SAT(cell) && !is_plat && !is_spike && !is_ramp) {
			if (!ceiling_collision && !just_landed) {
                has_right_collision = true;
				physics.acceleration.x = 0.0f;
				physics.velocity.x = 0.0f;
				physics.position.x += detector_mtv.x;
			}
            right_aabb_counter++;
        }

        ceiling_collision = false;

        if (jumpbox.SAT(cell) && !is_spike) {
            is_any_jump_colllision = !(is_plat && physics.velocity.y < 0.0f);
        }

        sync_components();
    }
}
