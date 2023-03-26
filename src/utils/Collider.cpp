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

        bounding_box.init();
        predictive_bounding_box.init();
        jumpbox.init();

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


        left_detector.left_offset = default_detector_width - 0.0001;
        right_detector.right_offset = default_detector_width - 0.0001;

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

        bounding_box.init();
        predictive_bounding_box.init();
        jumpbox.init();

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


        left_detector.left_offset = default_detector_width - 0.0001;
        right_detector.right_offset = default_detector_width - 0.0001;

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

    void Collider::handle_map_collision(const Shape& cell, bool is_ramp) {
        if (left_detector.SAT(cell) && physics.velocity.x < 0.01f && !is_ramp) {
            has_left_collision = true;
            physics.acceleration.x = 0.0f;
            physics.velocity.x = 0.0f;
            left_aabb_counter++;
        }
        if (right_detector.SAT(cell) && physics.velocity.x > -0.01f && !is_ramp) {
            has_right_collision = true;
            physics.acceleration.x = 0.0f;
            physics.velocity.x = 0.0f;
            right_aabb_counter++;
        }

        if (predictive_bounding_box.SAT(cell)) {
            is_any_colllision = true;
            //set mtv
            physics.mtv = predictive_bounding_box.testCollisionGetMTV(predictive_bounding_box, cell);


            if (physics.velocity.y > 3.0f) {
                physics.mtv.x = 0.0f;
            }
            //here, we can do MTV again with the player's predicted position based on velocity
            if (physics.velocity.y > -0.01f) {
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
            if (physics.velocity.y < -0.01f && abs(physics.mtv.y) > 0.001f) {
                float ydist = physics.position.y - predictive_bounding_box.shape_y;
                float correction = ydist + physics.mtv.y;
                physics.position.y += correction;
                physics.acceleration.y = 0.0f;
                physics.velocity.y *= -0.5;
                ceiling_collision = true;
            }

            //only for landing
            if (physics.velocity.y > 0.0f && !has_left_collision && !has_right_collision) {
                physics.acceleration.y = 0.0f;
                physics.velocity.y = 0.0f;
            }
            sync_components();

            physics.mtv = { 0.0f, 0.0f };
            just_collided = true;
            is_colliding_with_level = true;

        }
        if (jumpbox.SAT(cell)) {
            is_any_jump_colllision = true;
        }
    }
}
