//
//  Collider.cpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#include "Collider.hpp"
#include "../graphics/FLColor.hpp"

namespace shape {

    Collider::Collider() {

        physics = components::PhysicsComponent({ 0.9, 0.9 }, 1);

        dimensions = sf::Vector2<float>{ default_dim, default_dim };

        bounding_box.dimensions = dimensions;
        predictive_bounding_box.dimensions = dimensions;
        jumpbox.dimensions = sf::Vector2<float>(dimensions.x, default_jumpbox_height);
        left_detector.dimensions = sf::Vector2<float>(default_detector_width, default_detector_height);
        right_detector.dimensions = sf::Vector2<float>(default_detector_width, default_detector_height);
        hurtbox.dimensions = sf::Vector2<float>(dimensions.x / 2, dimensions.y / 2);

    }

    Collider::Collider(sf::Vector2<float> dim, sf::Vector2<float> start_pos) : dimensions(dim) {

        physics = components::PhysicsComponent({ 0.9, 0.9 }, 1);

        bounding_box.dimensions = dim;
        predictive_bounding_box.dimensions = dim;
        jumpbox.dimensions = sf::Vector2<float>(dim.x, default_jumpbox_height);
        left_detector.dimensions = sf::Vector2<float>(default_detector_width, default_detector_height);
        right_detector.dimensions = sf::Vector2<float>(default_detector_width, default_detector_height);
        hurtbox.dimensions = sf::Vector2<float>(dim.x / 2, dim.y / 2);

        bounding_box.position = start_pos;
        predictive_bounding_box.position = start_pos;
        jumpbox.position = sf::Vector2<float>(start_pos.x, start_pos.y + dim.y);
        left_detector.position = sf::Vector2<float>(start_pos.x - left_detector.dimensions.x, start_pos.y + default_detector_buffer);
        right_detector.position = sf::Vector2<float>(start_pos.x + dim.x, start_pos.y + default_detector_buffer);
        hurtbox.position = sf::Vector2<float>(start_pos.x + (dim.x / 2) - (hurtbox.dimensions.x / 2), start_pos.y + (dim.y / 2) - (hurtbox.dimensions.y / 2));

    }

    void Collider::sync_components() {

        if (left_aabb_counter == 0) {
            flags.reset(State::has_left_collision);
        }
        if (right_aabb_counter == 0) {
            flags.reset(State::has_right_collision);
        }
        if (flags.test(State::is_any_collision)) {
            flags.set(State::is_colliding_with_level);
        }
        else {
            flags.reset(State::is_colliding_with_level);
        }

        bounding_box.set_position(physics.position);
        predictive_bounding_box.set_position(physics.position + physics.velocity);
        jumpbox.set_position( sf::Vector2<float>{ physics.position.x, physics.position.y + dimensions.y} );
        left_detector.set_position(sf::Vector2<float>{ physics.position.x - default_detector_width, physics.position.y + default_detector_buffer});
        right_detector.set_position(sf::Vector2<float>{ physics.position.x + dimensions.x, physics.position.y + default_detector_buffer});
        hurtbox.set_position( sf::Vector2<float>(physics.position.x + (dimensions.x / 2) - (hurtbox.dimensions.x / 2), physics.position.y + (dimensions.y / 2) - (hurtbox.dimensions.y / 2)));

    }

    void Collider::handle_map_collision(const Shape& cell, lookup::TILE_TYPE tile_type) {

        //tile flags
        bool is_ramp = tile_type == lookup::TILE_TYPE::TILE_RAMP;
        bool is_plat = tile_type == lookup::TILE_TYPE::TILE_PLATFORM && (jumpbox.position.y > cell.position.y + 4 || physics.acceleration.y < 0.0f);
        bool is_spike = tile_type == lookup::TILE_TYPE::TILE_SPIKES;

        if (is_plat) { handle_platform_collision(cell); return; }
        if (is_spike) { handle_spike_collision(cell); return; }

		if (predictive_bounding_box.SAT(cell)) {

            flags.set(State::is_any_collision);
            if (is_ramp) { flags.set(State::on_ramp); }
			//set mtv
			physics.mtv = predictive_bounding_box.testCollisionGetMTV(predictive_bounding_box, cell);

			if (is_ramp && abs(physics.velocity.y) < 0.0001f) {
				physics.position.y += physics.mtv.y;
			}

			if (physics.velocity.y > 3.0f) {
				physics.mtv.x = 0.0f;
			}
			//here, we can do MTV resolution with the player's predicted position based on velocity
			bool y_collision{ false };
			auto vertical_buffer = predictive_bounding_box.position.y - predictive_bounding_box.dimensions.y;
			if (physics.velocity.y > -0.01f && vertical_buffer < cell.position.y) {
				if (physics.velocity.y > landed_threshold) {
					flags.set(State::just_landed);
				}
				float ydist = predictive_bounding_box.position.y - physics.position.y;
				float correction = ydist + physics.mtv.y;
				physics.position.y += correction;
				physics.velocity.y = 0.0f;
				physics.acceleration.y = 0.0f;
				y_collision = true;
				sync_components();
			}
			if (abs(physics.velocity.x) > 0.0f && !y_collision) {
				if (!is_ramp) {
					float xdist = predictive_bounding_box.position.x - physics.position.x;
					float correction = xdist + physics.mtv.x;
					physics.position.x += correction;
				}
				else { physics.position.x += physics.mtv.x; }
				sync_components();
			}

			//player hits the ceiling
			if (!flags.test(State::grounded)) {
				if (!is_ramp) {
					float ydist = physics.position.y - predictive_bounding_box.position.y;
					float correction = ydist + physics.mtv.y;
					physics.position.y += correction;
				}
				else { physics.position.y += physics.mtv.y; }
				physics.acceleration.y = 0.0f;
				physics.velocity.y *= -0.5f;
				flags.reset(State::ceiling_collision);
				sync_components();
			}

			//only for landing
			if (physics.velocity.y > 0.0f && !flags.test(State::has_left_collision) && !flags.test(State::has_right_collision)) {
				physics.acceleration.y = 0.0f;
				physics.velocity.y = 0.0f;
			}

			physics.mtv = { 0.0f, 0.0f };
			flags.set(State::just_collided);
			flags.set(State::is_colliding_with_level);

		}

		//bool higher_than_cell_top = (cell.position.y) - (physics.position.y + dimensions.y) > 0.1f;

		//if (higher_than_cell_top) {
		float y_dist = cell.vertices[0].y - left_detector.vertices[2].y;
		sf::Vector2<float> detector_mtv = left_detector.testCollisionGetMTV(left_detector, cell);

		bool left_collision = physics.velocity.x < 0.0f;

		if (left_detector.SAT(cell) && left_collision && !is_plat && !is_spike && !is_ramp) {
			if (!flags.test(State::ceiling_collision) && !flags.test(State::just_landed)) {
				flags.set(State::has_left_collision);
				physics.acceleration.x = 0.0f;
				physics.velocity.x = 0.0f;
				physics.position.x += detector_mtv.x;
			}
			left_aabb_counter++;
		}
		detector_mtv = right_detector.testCollisionGetMTV(right_detector, cell);
		if (right_detector.SAT(cell) && !left_collision && !is_plat && !is_spike && !is_ramp) {
			if (!flags.test(State::ceiling_collision) && !flags.test(State::just_landed)) {
				flags.set(State::has_right_collision);
				physics.acceleration.x = 0.0f;
				physics.velocity.x = 0.0f;
				physics.position.x += detector_mtv.x;
			}
			right_aabb_counter++;
		}
		//}

		flags.reset(State::ceiling_collision);

		if (jumpbox.SAT(cell) && !is_spike) {
            flags.set(State::grounded);
            flags.set(State::is_any_jump_collision);
			/*if (!is_plat) { flags.set(State::grounded); }
			(is_plat && jumpbox.position.y > cell.position.y && abs(jumpbox.position.y - cell.position.y) < 16.0f) ? flags.reset(State::is_any_jump_collision) : flags.set(State::is_any_jump_collision);
			(is_plat && jumpbox.position.y < cell.position.y + 4) ? flags.reset(State::grounded) : flags.set(State::grounded);*/
		} else {
			flags.reset(State::grounded);
		}

		sync_components();
	}


    void Collider::handle_platform_collision(const Shape& cell) {

    }

	void Collider::handle_spike_collision(const Shape& cell) {
		if (hurtbox.SAT(cell)) {
			spike_trigger = true;
		}
	}

    void Collider::update() {
        if (!flags.test(State::is_colliding_with_level)) { physics.mtv = { 0.0f, 0.0f }; }
        flags.reset(State::just_collided);
        physics.gravity = flags.test(State::grounded) ? 0.0f : stats.GRAV;
    }

    

    void Collider::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
        box.setSize(dimensions);
        box.setPosition(bounding_box.position.x - cam.x, bounding_box.position.y - cam.y);
        box.setFillColor(sf::Color{50, 60, 230, 80});
        box.setOutlineColor(flcolor::white);
        box.setOutlineThickness(-1);
        win.draw(box);
        box.setSize(dimensions);
        box.setPosition(predictive_bounding_box.position.x - cam.x, predictive_bounding_box.position.y - cam.y);
        if (flags.test(State::has_left_collision)) {
            box.setFillColor(sf::Color{ 50, 230, 50, 200 });
        } else if(flags.test(State::has_right_collision)) {
            box.setFillColor(sf::Color{ 50, 60, 230, 200 });
        } else {
            box.setFillColor(sf::Color{ 230, 230, 10, 70 });
        }
        box.setOutlineColor(flcolor::fucshia);
        box.setOutlineThickness(-1);
        win.draw(box);
        box.setSize(sf::Vector2<float>{(float)left_detector.dimensions.x, (float)left_detector.dimensions.y});
        box.setPosition(left_detector.position.x - cam.x, left_detector.position.y - cam.y);
        box.setFillColor(sf::Color{ 50, 60, 130, 80 });
        win.draw(box);
        box.setSize(sf::Vector2<float>{(float)right_detector.dimensions.x, (float)right_detector.dimensions.y});
        box.setPosition(right_detector.position.x - cam.x, right_detector.position.y - cam.y);
        box.setFillColor(sf::Color{ 50, 60, 130, 80 });
        win.draw(box);
        box.setSize(sf::Vector2<float>{(float)hurtbox.dimensions.x, (float)hurtbox.dimensions.y});
        box.setPosition(hurtbox.position.x - cam.x, hurtbox.position.y - cam.y);
        box.setFillColor(flcolor::goldenrod);
        win.draw(box);

    }
    void Collider::reset() {
        flags.reset(State::is_any_jump_collision);
        flags.reset(State::is_any_collision);
        left_aabb_counter = 0;
        right_aabb_counter = 0;
        flags = {};
    }
    void Collider::reset_ground_flags() {

        if (flags.test(shape::State::is_any_jump_collision)) {
            flags.set(shape::State::grounded);
        }
        else {
            flags.reset(shape::State::grounded);
        }

    }
    bool Collider::on_ramp() {
        return flags.test(State::on_ramp);
    }
}
