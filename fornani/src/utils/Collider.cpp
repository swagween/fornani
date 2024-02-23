
#include "Collider.hpp"
#include "../graphics/FLColor.hpp"
#include "../setup/ServiceLocator.hpp"

namespace shape {

Collider::Collider() {

	physics = components::PhysicsComponent({0.9, 0.9}, 1);

	dimensions = sf::Vector2<float>{default_dim, default_dim};

	bounding_box.dimensions = dimensions;
	predictive_bounding_box.dimensions = dimensions;
	jumpbox.dimensions = sf::Vector2<float>(dimensions.x, default_jumpbox_height);
	left_detector.dimensions = sf::Vector2<float>(default_detector_width, default_detector_height);
	right_detector.dimensions = sf::Vector2<float>(default_detector_width, default_detector_height);
	top_detector.dimensions = sf::Vector2<float>(dimensions.x, default_detector_width);
	bottom_detector.dimensions = sf::Vector2<float>(dimensions.x, default_detector_width);
	hurtbox.dimensions = sf::Vector2<float>(dimensions.x / 2, dimensions.y / 2);
}

Collider::Collider(sf::Vector2<float> dim, sf::Vector2<float> start_pos) : dimensions(dim) {

	physics = components::PhysicsComponent({0.9, 0.9}, 1);

	bounding_box.dimensions = dim;
	predictive_bounding_box.dimensions = dim;
	jumpbox.dimensions = sf::Vector2<float>(dim.x, default_jumpbox_height);
	left_detector.dimensions = sf::Vector2<float>(default_detector_width, dim.y - 1);
	right_detector.dimensions = sf::Vector2<float>(default_detector_width, dim.y - 1);
	top_detector.dimensions = sf::Vector2<float>(dim.x, default_detector_width);
	bottom_detector.dimensions = sf::Vector2<float>(dim.x, default_detector_width);
	hurtbox.dimensions = sf::Vector2<float>(dim.x / 2, dim.y / 2);

	detector_buffer = (dim.y - left_detector.dimensions.y) / 2;

	bounding_box.position = start_pos;
	predictive_bounding_box.position = start_pos;
	jumpbox.position = sf::Vector2<float>(start_pos.x, start_pos.y + dim.y);
	left_detector.position = sf::Vector2<float>(start_pos.x - left_detector.dimensions.x, start_pos.y + detector_buffer);
	right_detector.position = sf::Vector2<float>(start_pos.x + dim.x, start_pos.y + detector_buffer);
	top_detector.position = sf::Vector2<float>(start_pos.x, start_pos.y - top_detector.dimensions.y);
	bottom_detector.position = sf::Vector2<float>(start_pos.x, start_pos.y + dim.y - bottom_detector.dimensions.y);

	hurtbox.position = sf::Vector2<float>(start_pos.x + (dim.x / 2) - (hurtbox.dimensions.x / 2), start_pos.y + (dim.y / 2) - (hurtbox.dimensions.y / 2));
}

void Collider::sync_components() {

	if (flags.test(State::is_any_collision)) {
		flags.set(State::is_colliding_with_level);
	} else {
		flags.reset(State::is_colliding_with_level);
	}

	detector_buffer = 0;

	bounding_box.set_position(physics.position);
	predictive_bounding_box.set_position(physics.position + physics.velocity);
	jumpbox.set_position(sf::Vector2<float>{physics.position.x, physics.position.y + dimensions.y});
	left_detector.set_position(sf::Vector2<float>{physics.position.x - left_detector.dimensions.x, physics.position.y + detector_buffer});
	right_detector.set_position(sf::Vector2<float>{physics.position.x + dimensions.x, physics.position.y + detector_buffer});
	hurtbox.set_position(sf::Vector2<float>(physics.position.x + (dimensions.x / 2) - (hurtbox.dimensions.x / 2), physics.position.y + (dimensions.y / 2) - (hurtbox.dimensions.y / 2)));
	top_detector.set_position(sf::Vector2<float>(physics.position.x, physics.position.y - top_detector.dimensions.y));
	bottom_detector.set_position(sf::Vector2<float>(physics.position.x, physics.position.y + dimensions.y - bottom_detector.dimensions.y));
}

void Collider::handle_map_collision(Shape const& cell, lookup::TILE_TYPE tile_type) {

	// tile flags
	bool is_ground_ramp = tile_type == lookup::TILE_TYPE::TILE_GROUND_RAMP;
	bool is_ceiling_ramp = tile_type == lookup::TILE_TYPE::TILE_CEILING_RAMP;
	bool is_plat = tile_type == lookup::TILE_TYPE::TILE_PLATFORM && (jumpbox.position.y > cell.position.y + 4 || physics.acceleration.y < 0.0f);
	bool is_spike = tile_type == lookup::TILE_TYPE::TILE_SPIKES;

	if (is_plat) {
		handle_platform_collision(cell);
		return;
	}
	if (is_spike) {
		handle_spike_collision(cell);
		return;
	}

	if (predictive_bounding_box.SAT(cell)) {

		// just for animation purposes
		if (physics.velocity.y > landed_threshold) { flags.set(State::just_landed); }

		// set collision flags
		flags.set(State::is_any_collision);
		if (is_ground_ramp) { flags.set(State::on_ramp); }

		// set mtv
		physics.mtv = predictive_bounding_box.testCollisionGetMTV(predictive_bounding_box, cell);

		// handle top detector collisions before general case
		sf::Vector2<float> detector_mtv = top_detector.testCollisionGetMTV(top_detector, cell);
		detector_mtv = top_detector.testCollisionGetMTV(top_detector, cell);
		if (top_detector.SAT(cell) && !is_plat && !is_spike) {
			flags.set(State::has_top_collision);
			flags.set(State::ceiling_collision);
			physics.acceleration.y = 0.0f;
			physics.velocity.y = 0.0f;
			physics.position.y += abs(detector_mtv.y);
		}
		detector_mtv = bottom_detector.testCollisionGetMTV(bottom_detector, cell);
		if (bottom_detector.SAT(cell) && is_ground_ramp) {
			flags.set(State::has_bottom_collision);
			physics.acceleration.y = 0.0f;
			physics.velocity.y = 0.0f;
			physics.position.y -= abs(detector_mtv.y);
			svc::floatReadoutLocator.get() = detector_mtv.y;
		}

		// ramp-specific stuff
		// i'm being very long-winded for clarity, because collision resolution is very complicated
		// all this is only for when the player lands on a ground ramp. we don't correct the x position if the player walks up a ramp!
		bool falls_onto_ramp = is_ground_ramp && physics.velocity.y > 0.f;
		bool walks_up_ramp = is_ground_ramp;
		bool jumps_into_ramp = is_ceiling_ramp && physics.velocity.y < 0.f;

		if (falls_onto_ramp) {
			auto temp_mtv = bounding_box.testCollisionGetMTV(bounding_box, cell);
			bool one_overlaps = !(abs(temp_mtv.x) > 0.f) && !(abs(temp_mtv.y) > 0.f);

			// case 1, where the bounding box has some distance from the ramp
			if (one_overlaps) {
				auto xdist = predictive_bounding_box.position.x - physics.position.x;
				auto correction = xdist - physics.mtv.x;

				physics.position.x += correction;

				auto ydist = predictive_bounding_box.position.y - physics.position.y;
				correction = ydist - physics.mtv.y;
				physics.position.y -= abs(correction);
				physics.velocity.y = 0.0f;
				physics.acceleration.y = 0.0f;

				// case 2, where both the bounding box and the predictive bounding box overlap the ramp
			} else {
				physics.position.x += temp_mtv.x;
				physics.position.y += temp_mtv.y;
			}
		}

		// just walking up the ramp, so we only correct y position
		if (walks_up_ramp && !flags.test(State::has_top_collision)) {
			physics.position.y += physics.mtv.y;
			// still zero this because of gravity
			physics.velocity.y = 0.0f;
			physics.acceleration.y = 0.0f;
		}

		// and finally, a player jumps into a ramp. it's very similar to landing on a ramp, it just has different signs
		if (jumps_into_ramp) {
			auto temp_mtv = bounding_box.testCollisionGetMTV(bounding_box, cell);
			bool one_overlaps = !(abs(temp_mtv.x) > 0.f) && !(abs(temp_mtv.y) > 0.f);

			// case 1, where the bounding box has some distance from the ramp
			if (one_overlaps) {
				auto xdist = physics.position.x - predictive_bounding_box.position.x;
				auto correction = xdist - physics.mtv.x;

				physics.position.x -= correction * 0.9f;

				auto ydist = physics.position.y - predictive_bounding_box.position.y;
				correction = ydist - physics.mtv.y;
				physics.position.y -= correction;
				physics.velocity.y = 0.0f;
				physics.acceleration.y = 0.0f;

				// case 2, where both the bounding box and the predictive bounding box overlap the ramp
			} else {
				physics.position.x += temp_mtv.x;
				physics.position.y += temp_mtv.y;
			}
			flags.set(State::ceiling_collision);
		}

		// basic collision resolution
		// do this after because we need the velocity to detect how the collider interacts with the ramp
		bool other_collisions = !is_ceiling_ramp && !is_ground_ramp && !walks_up_ramp;
		if (!is_ceiling_ramp && !flags.test(State::has_top_collision)) {
			auto ydist = predictive_bounding_box.position.y - physics.position.y;
			auto correction = ydist + physics.mtv.y;
			physics.position.y += correction;
			physics.velocity.y = 0.0f;
			physics.acceleration.y = 0.0f;
		}

		// do this again in case there is an unusual outcome from the above checks
		auto temp_mtv = bounding_box.testCollisionGetMTV(bounding_box, cell);
		physics.position.x += temp_mtv.x;
		physics.position.y += temp_mtv.y;

		physics.mtv = {0.0f, 0.0f};
		flags.set(State::just_collided);
		flags.set(State::is_colliding_with_level);
	}

	// bool higher_than_cell_top = (cell.position.y) - (physics.position.y + dimensions.y) > 0.1f;

	// if (higher_than_cell_top) {
	float y_dist = cell.vertices[0].y - left_detector.vertices[2].y;

	sf::Vector2<float> detector_mtv = left_detector.testCollisionGetMTV(left_detector, cell);
	bool left_collision = physics.velocity.x < 0.0f;
	if (left_detector.SAT(cell) && left_collision && !is_plat && !is_spike) {
		if (!flags.test(State::just_landed) && (!is_ground_ramp || is_ceiling_ramp)) {
			flags.set(State::has_left_collision);
			physics.acceleration.x = 0.0f;
			physics.velocity.x = 0.0f;
			physics.position.x += detector_mtv.x;
		}
	}
	detector_mtv = right_detector.testCollisionGetMTV(right_detector, cell);
	if (right_detector.SAT(cell) && !left_collision && !is_plat && !is_spike) {
		if (!flags.test(State::just_landed) && (!is_ground_ramp || is_ceiling_ramp)) {
			flags.set(State::has_right_collision);
			physics.acceleration.x = 0.0f;
			physics.velocity.x = 0.0f;
			physics.position.x += detector_mtv.x;
		}
	}

	flags.reset(State::ceiling_collision);

	if (jumpbox.SAT(cell) && !is_spike) {
		flags.set(State::grounded);
		flags.set(State::is_any_jump_collision);
	} else {
		flags.reset(State::grounded);
	}

	sync_components();
}

void Collider::handle_platform_collision(Shape const& cell) {}

void Collider::handle_spike_collision(Shape const& cell) {
	if (hurtbox.SAT(cell)) { spike_trigger = true; }
}

void Collider::handle_collider_collision(Shape const& collider) {

	if (predictive_bounding_box.SAT(collider)) {

		// just for animation purposes
		if (physics.velocity.y > landed_threshold) { flags.set(State::just_landed); }
		// set mtv
		physics.mtv = predictive_bounding_box.testCollisionGetMTV(predictive_bounding_box, collider);

		// handle top detector collisions before general case
		sf::Vector2<float> detector_mtv = top_detector.testCollisionGetMTV(top_detector, collider);
		detector_mtv = top_detector.testCollisionGetMTV(top_detector, collider);
		if (top_detector.SAT(collider)) {
			flags.set(State::has_top_collision);
			flags.set(State::ceiling_collision);
			physics.acceleration.y = 0.0f;
			physics.velocity.y = 0.0f;
			physics.position.y += abs(detector_mtv.y);
		}
		detector_mtv = bottom_detector.testCollisionGetMTV(bottom_detector, collider);
		if (bottom_detector.SAT(collider)) {
			flags.set(State::has_bottom_collision);
			physics.acceleration.y = 0.0f;
			physics.velocity.y = 0.0f;
			physics.position.y -= abs(detector_mtv.y);
			svc::floatReadoutLocator.get() = detector_mtv.y;
		}

		auto ydist = predictive_bounding_box.position.y - physics.position.y;
		auto correction = ydist + physics.mtv.y;
		physics.position.y += correction;
		physics.velocity.y = 0.0f;
		physics.acceleration.y = 0.0f;
		sync_components();

		// do this again in case there is an unusual outcome from the above checks
		auto temp_mtv = bounding_box.testCollisionGetMTV(bounding_box, collider);
		physics.position.x += temp_mtv.x;
		physics.position.y += temp_mtv.y;

		physics.mtv = {0.0f, 0.0f};
		flags.set(State::just_collided);
		flags.set(State::is_colliding_with_level);
	}

	sf::Vector2<float> detector_mtv = left_detector.testCollisionGetMTV(left_detector, collider);
	bool left_collision = physics.velocity.x < 0.0f;
	if (left_detector.SAT(collider) && left_collision) {
		if (!flags.test(State::just_landed)) {
			flags.set(State::has_left_collision);
			physics.acceleration.x = 0.0f;
			physics.velocity.x = 0.0f;
			physics.position.x += detector_mtv.x;
		}
	}

	detector_mtv = right_detector.testCollisionGetMTV(right_detector, collider);
	if (right_detector.SAT(collider) && !left_collision) {
		if (!flags.test(State::just_landed)) {
			flags.set(State::has_right_collision);
			physics.acceleration.x = 0.0f;
			physics.velocity.x = 0.0f;
			physics.position.x += detector_mtv.x;
		}
	}
	if (jumpbox.SAT(collider)) {
		flags.set(State::grounded);
		flags.set(State::is_any_jump_collision);
	} else {
		flags.reset(State::grounded);
	}
}

void Collider::update() {
	if (!flags.test(State::is_colliding_with_level)) { physics.mtv = {0.0f, 0.0f}; }
	flags.reset(State::just_collided);
	physics.gravity = flags.test(State::grounded) ? 0.0f : stats.GRAV;
	flags.test(State::grounded) ? physics.flags.set(components::State::grounded) : physics.flags.reset(components::State::grounded);
}

void Collider::render(sf::RenderWindow& win, sf::Vector2<float> cam) {

	// draw predictive bounding box
	box.setSize(dimensions);
	box.setPosition(predictive_bounding_box.position.x - cam.x, predictive_bounding_box.position.y - cam.y);
	box.setOutlineColor(flcolor::fucshia);
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw bounding box
	box.setSize(dimensions);
	box.setPosition(bounding_box.position.x - cam.x, bounding_box.position.y - cam.y);
	box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(sf::Color{255, 255, 255, 80});
	box.setOutlineThickness(-1);
	flags.test(State::is_colliding_with_level) ? box.setFillColor(flcolor::goldenrod) : box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw left detector
	box.setSize(sf::Vector2<float>{(float)left_detector.dimensions.x, (float)left_detector.dimensions.y});
	box.setPosition(left_detector.position.x - cam.x, left_detector.position.y - cam.y);
	flags.test(State::has_left_collision) ? box.setFillColor(flcolor::goldenrod) : box.setFillColor(sf::Color{50, 60, 130, 80});
	win.draw(box);

	// draw right detector
	box.setSize(sf::Vector2<float>{(float)right_detector.dimensions.x, (float)right_detector.dimensions.y});
	box.setPosition(right_detector.position.x - cam.x, right_detector.position.y - cam.y);
	flags.test(State::has_right_collision) ? box.setFillColor(flcolor::goldenrod) : box.setFillColor(sf::Color{50, 60, 130, 80});
	win.draw(box);

	// draw top detector
	box.setSize(sf::Vector2<float>{(float)top_detector.dimensions.x, (float)top_detector.dimensions.y});
	box.setPosition(top_detector.position.x - cam.x, top_detector.position.y - cam.y);
	flags.test(State::has_top_collision) ? box.setFillColor(flcolor::goldenrod) : box.setFillColor(sf::Color{50, 60, 130, 80});
	win.draw(box);

	// draw bottom detector
	box.setSize(sf::Vector2<float>{(float)bottom_detector.dimensions.x, (float)bottom_detector.dimensions.y});
	box.setPosition(bottom_detector.position.x - cam.x, bottom_detector.position.y - cam.y);
	flags.test(State::has_bottom_collision) ? box.setFillColor(flcolor::goldenrod) : box.setFillColor(sf::Color{50, 60, 130, 80});
	win.draw(box);

	// draw hurtbox
	box.setSize(sf::Vector2<float>{(float)hurtbox.dimensions.x, (float)hurtbox.dimensions.y});
	box.setPosition(hurtbox.position.x - cam.x, hurtbox.position.y - cam.y);
	box.setFillColor(flcolor::goldenrod);
	// win.draw(box);
}
void Collider::reset() { flags = {}; }
void Collider::reset_ground_flags() {

	if (flags.test(shape::State::is_any_jump_collision)) {
		flags.set(shape::State::grounded);
	} else {
		flags.reset(shape::State::grounded);
	}
}
bool Collider::on_ramp() { return flags.test(State::on_ramp); }

} // namespace shape
