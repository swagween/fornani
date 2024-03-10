
#include "Collider.hpp"
#include "../graphics/FLColor.hpp"
#include "../setup/ServiceLocator.hpp"

namespace shape {

Collider::Collider() {

	physics = components::PhysicsComponent({0.9, 0.9}, 1);

	dimensions = sf::Vector2<float>{default_dim, default_dim};

	bounding_box.dimensions = dimensions;
	predictive_vertical.dimensions = dimensions;
	predictive_horizontal.dimensions = dimensions;
	predictive_combined.dimensions = dimensions;
	vicinity.dimensions.x = dimensions.x + 2 * vicinity_pad;
	vicinity.dimensions.y = dimensions.y + 2 * vicinity_pad;
	jumpbox.dimensions = sf::Vector2<float>(dimensions.x, default_jumpbox_height);
	hurtbox.dimensions = sf::Vector2<float>(dimensions.x / 2, dimensions.y / 2);
}

Collider::Collider(sf::Vector2<float> dim, sf::Vector2<float> start_pos) : dimensions(dim) {

	physics = components::PhysicsComponent({0.9, 0.9}, 1);

	bounding_box.dimensions = dim;
	predictive_vertical.dimensions = dim;
	predictive_horizontal.dimensions = dim;
	predictive_combined.dimensions = dim;
	vicinity.dimensions.x = dim.x + 2 * vicinity_pad;
	vicinity.dimensions.y = dim.y + 2 * vicinity_pad;
	jumpbox.dimensions = sf::Vector2<float>(dim.x, default_jumpbox_height);
	hurtbox.dimensions = sf::Vector2<float>(dim.x / 2, dim.y / 2);

	bounding_box.position = start_pos;
	predictive_vertical.position = start_pos;
	predictive_horizontal.position = start_pos;
	predictive_combined.position = start_pos;
	jumpbox.position = sf::Vector2<float>(start_pos.x, start_pos.y + dim.y);

	hurtbox.position = sf::Vector2<float>(start_pos.x + (dim.x / 2) - (hurtbox.dimensions.x / 2), start_pos.y + (dim.y / 2) - (hurtbox.dimensions.y / 2));
}

void Collider::sync_components() {

	bounding_box.set_position(physics.position);
	vicinity.dimensions.x = dimensions.x + 2 * vicinity_pad;
	vicinity.dimensions.y = dimensions.y + 2 * vicinity_pad;
	predictive_vertical.dimensions.x = dimensions.x - 2 * vertical_detector_buffer;
	predictive_vertical.dimensions.y = dimensions.y + 2 * vertical_detector_buffer;
	predictive_horizontal.dimensions.x = dimensions.x + 2 * horizontal_detector_buffer;
	predictive_horizontal.dimensions.y = dimensions.y - 3 * horizontal_detector_buffer;
	predictive_combined.dimensions = dimensions;
	vicinity.set_position(sf::Vector2<float>{physics.position.x - vicinity_pad + physics.velocity.x, physics.position.y - vicinity_pad + physics.velocity.y});
	predictive_vertical.set_position(sf::Vector2<float>{physics.position.x + vertical_detector_buffer, physics.position.y - vertical_detector_buffer + physics.velocity.y});
	predictive_horizontal.set_position(sf::Vector2<float>{physics.position.x - horizontal_detector_buffer + physics.velocity.x, physics.position.y + horizontal_detector_buffer});
	predictive_combined.set_position(sf::Vector2<float>{physics.position.x + physics.velocity.x, physics.position.y + physics.velocity.y});
	jumpbox.set_position(sf::Vector2<float>{physics.position.x, physics.position.y + dimensions.y});
	hurtbox.set_position(sf::Vector2<float>(physics.position.x + (dimensions.x / 2) - (hurtbox.dimensions.x / 2), physics.position.y + (dimensions.y / 2) - (hurtbox.dimensions.y / 2)));
}

void Collider::handle_map_collision(Shape const& cell, lookup::TILE_TYPE tile_type) {

	collision_flags = {};

	// tile flags
	bool is_ground_ramp = tile_type == lookup::TILE_TYPE::TILE_GROUND_RAMP;
	bool is_ceiling_ramp = tile_type == lookup::TILE_TYPE::TILE_CEILING_RAMP;
	bool is_plat = tile_type == lookup::TILE_TYPE::TILE_PLATFORM && (jumpbox.position.y > cell.position.y + 4 || physics.acceleration.y < 0.0f);
	bool is_spike = tile_type == lookup::TILE_TYPE::TILE_SPIKES;
	bool is_ramp = (is_ground_ramp || is_ceiling_ramp);

	// special tile types
	if (is_plat) {
		handle_platform_collision(cell);
		return;
	}
	if (is_spike) {
		handle_spike_collision(cell);
		return;
	}

	// store all four mtvs
	auto combined_mtv = predictive_combined.testCollisionGetMTV(predictive_combined, cell);
	auto vert_mtv = predictive_vertical.testCollisionGetMTV(predictive_vertical, cell);
	auto horiz_mtv = predictive_horizontal.testCollisionGetMTV(predictive_horizontal, cell);
	auto actual_mtv = bounding_box.testCollisionGetMTV(bounding_box, cell);

	float vert_threshold = 5.5f; // for landing
	// let's first settle all actual block collisions
	if (!is_ramp) {
		bool corner_collision{true};
		if (predictive_vertical.overlaps(cell)) {
			vert_mtv.y < 0.f ? collision_flags.set(Collision::has_bottom_collision) : collision_flags.set(Collision::has_top_collision);
			if (collision_flags.test(Collision::has_bottom_collision) && physics.velocity.y > vert_threshold) { flags.set(State::just_landed); } // for landing sound
			corner_collision = false;
			correct_y(vert_mtv);
		}
		if (predictive_horizontal.overlaps(cell)) {
			horiz_mtv.x > 0.f ? collision_flags.set(Collision::has_left_collision) : collision_flags.set(Collision::has_right_collision);
			corner_collision = false;
			dash_flags.set(Dash::dash_cancel_collision);
			correct_x(horiz_mtv);
		}
		if (predictive_combined.overlaps(cell) && corner_collision) {
			collision_flags.set(Collision::any_collision);
			dash_flags.set(Dash::dash_cancel_collision);
			correct_corner(combined_mtv);
		}
	}

	// now let's settle ramp collisions. remember, the collider has already been resolved from any previous cell collision
	if (is_ramp) {
		bool falls_onto = is_ground_ramp && physics.velocity.y > vert_threshold;
		bool jumps_into = physics.velocity.y < vert_threshold;
		// ground ramp
		// only handle ramp collisions if the bounding_box is colliding with it
		if (bounding_box.SAT(cell)) {
			if (is_ground_ramp) {
				if (actual_mtv.y < 0.f) { physics.position.y += actual_mtv.y; }
				// still zero this because of gravity
				if (!movement_flags.test(Movement::jumping)) {
					physics.velocity.y = 0.0f;
					physics.acceleration.y = 0.0f;
				}
			}
			if (is_ceiling_ramp) { correct_x_y(actual_mtv); }
			// cancel dash
			dash_flags.set(Dash::dash_cancel_collision);
		}
		// we also need to check if the predictive bounding box is colliding a ramp, just to deal with falling/jumping onto and into ramps
		if (predictive_combined.SAT(cell)) {
			if (falls_onto) {
				correct_x_y(combined_mtv);
				flags.set(State::just_landed);
			}
			if (is_ceiling_ramp) {
				if (jumps_into) {
					if (movement_flags.test(Movement::dashing)) {
						correct_y(combined_mtv);
					} else {
						collision_flags.set(Collision::any_collision);
						correct_y(combined_mtv + sf::Vector2<float>{8.f, 8.f}); // to prevent player gliding down ceiling ramps
					}
				}
			}
		}
	}

	if (jumpbox.SAT(cell)) {
		flags.set(State::grounded);
		flags.set(State::is_any_jump_collision);
	} else {
		flags.reset(State::grounded);
	}

	movement_flags.reset(Movement::dashing);

	sync_components();
}

void Collider::correct_x(sf::Vector2<float> mtv) {
	auto xdist = predictive_horizontal.position.x + horizontal_detector_buffer - physics.position.x;
	auto correction = xdist + mtv.x;
	physics.position.x += correction;
	physics.acceleration.x = 0.0f;
	physics.velocity.x = 0.0f;
}

void Collider::correct_y(sf::Vector2<float> mtv) {
	auto ydist = predictive_vertical.position.y + vertical_detector_buffer - physics.position.y;
	auto correction = ydist + mtv.y;
	physics.position.y += correction;
	physics.acceleration.y = 0.0f;
	physics.velocity.y = 0.0f;
}

void Collider::correct_x_y(sf::Vector2<float> mtv) {
	auto xdist = predictive_combined.position.x - physics.position.x;
	auto correction = xdist + mtv.x;
	physics.position.x += correction;
	physics.acceleration.x = 0.0f;
	physics.velocity.x = 0.0f;
	auto ydist = predictive_combined.position.y - physics.position.y;
	correction = ydist + mtv.y;
	physics.position.y += correction;
	physics.acceleration.y = 0.0f;
	physics.velocity.y = 0.0f;
}

void Collider::correct_corner(sf::Vector2<float> mtv) {
	auto ydist = predictive_vertical.position.y - physics.position.y;
	auto correction = ydist + mtv.y;
	physics.position.y += correction;
	physics.acceleration.y = 0.0f;
	physics.velocity.y = 0.0f;
}

void Collider::handle_platform_collision(Shape const& cell) {}

void Collider::handle_spike_collision(Shape const& cell) {
	if (hurtbox.overlaps(cell)) { spike_trigger = true; }
}

void Collider::handle_collider_collision(Shape const& collider) {

	collision_flags = {};

	// store all four mtvs
	auto combined_mtv = predictive_combined.testCollisionGetMTV(predictive_combined, collider);
	auto vert_mtv = predictive_vertical.testCollisionGetMTV(predictive_vertical, collider);
	auto horiz_mtv = predictive_horizontal.testCollisionGetMTV(predictive_horizontal, collider);
	auto actual_mtv = bounding_box.testCollisionGetMTV(bounding_box, collider);

	float vert_threshold = 5.5f;
	bool corner_collision{true};
	if (predictive_vertical.overlaps(collider)) {
		vert_mtv.y < 0.f ? collision_flags.set(Collision::has_bottom_collision) : collision_flags.set(Collision::has_top_collision);
		if (collision_flags.test(Collision::has_bottom_collision) && physics.velocity.y > vert_threshold) { flags.set(State::just_landed); } // for landing sound
		corner_collision = false;
		correct_y(vert_mtv);
	}
	if (predictive_horizontal.overlaps(collider)) {
		horiz_mtv.x > 0.f ? collision_flags.set(Collision::has_left_collision) : collision_flags.set(Collision::has_right_collision);
		corner_collision = false;
		dash_flags.set(Dash::dash_cancel_collision);
		correct_x(horiz_mtv);
	}
	if (predictive_combined.overlaps(collider) && corner_collision) {
		collision_flags.set(Collision::any_collision);
		dash_flags.set(Dash::dash_cancel_collision);
		correct_corner(combined_mtv);
	}
	if (jumpbox.SAT(collider)) {
		flags.set(State::grounded);
		flags.set(State::is_any_jump_collision);
	} else {
		flags.reset(State::grounded);
	}

	movement_flags.reset(Movement::dashing);

	sync_components();
}

void Collider::update() {
	// if (!flags.test(State::is_colliding_with_level)) { physics.mtv = {0.0f, 0.0f}; }
	flags.reset(State::just_collided);
	physics.gravity = flags.test(State::grounded) ? 0.0f : stats.GRAV;
	flags.test(State::grounded) ? physics.flags.set(components::State::grounded) : physics.flags.reset(components::State::grounded);
}

void Collider::render(sf::RenderWindow& win, sf::Vector2<float> cam) {

	// draw bounding box
	box.setSize(dimensions);
	box.setPosition(bounding_box.position.x - cam.x, bounding_box.position.y - cam.y);
	box.setFillColor(sf::Color{200, 150, 255, 20});
	box.setOutlineColor(sf::Color{255, 255, 255, 255});
	box.setOutlineThickness(0);
	// flags.test(State::is_colliding_with_level) ? box.setFillColor(sf::Color{90, 100, 20, 60}) : box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw predictive vertical
	box.setSize(predictive_vertical.dimensions);
	box.setPosition(predictive_vertical.position.x - cam.x, predictive_vertical.position.y - cam.y);
	box.setOutlineColor(sf::Color{255, 0, 0, 160});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw predictive horizontal
	box.setSize(predictive_horizontal.dimensions);
	box.setPosition(predictive_horizontal.position.x - cam.x, predictive_horizontal.position.y - cam.y);
	box.setOutlineColor(sf::Color{0, 0, 255, 160});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw predictive combined
	box.setSize(predictive_combined.dimensions);
	box.setPosition(predictive_combined.position.x - cam.x, predictive_combined.position.y - cam.y);
	box.setOutlineColor(sf::Color{255, 255, 255, 80});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw jump box
	box.setSize(jumpbox.dimensions);
	box.setPosition(jumpbox.position.x - cam.x, jumpbox.position.y - cam.y);
	box.setFillColor(flcolor::navy_blue);
	box.setOutlineColor(sf::Color::Transparent);
	flags.test(State::is_any_jump_collision) ? box.setFillColor(flcolor::blue) : box.setFillColor(flcolor::navy_blue);
	win.draw(box);

	// draw hurtbox
	box.setSize(sf::Vector2<float>{(float)hurtbox.dimensions.x, (float)hurtbox.dimensions.y});
	box.setPosition(hurtbox.position.x - cam.x, hurtbox.position.y - cam.y);
	box.setFillColor(flcolor::goldenrod);
	// win.draw(box);

	// draw vicinity
	box.setSize(sf::Vector2<float>{(float)vicinity.dimensions.x, (float)vicinity.dimensions.y});
	box.setPosition(vicinity.position.x - cam.x, vicinity.position.y - cam.y);
	box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(flcolor::dark_orange);
	box.setOutlineThickness(-1);
	win.draw(box);

	// draw physics position
	box.setSize({2.f, 2.f});
	box.setPosition(physics.position.x - cam.x, physics.position.y - cam.y);
	box.setFillColor(flcolor::green);
	box.setOutlineThickness(0);
	win.draw(box);
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

bool Collider::has_horizontal_collision() const { return collision_flags.test(Collision::has_left_collision) || collision_flags.test(Collision::has_right_collision); }

bool Collider::has_left_collision() const { return collision_flags.test(Collision::has_left_collision); }

bool Collider::has_right_collision() const { return collision_flags.test(Collision::has_right_collision); }

bool Collider::has_vertical_collision() const { return collision_flags.test(Collision::has_top_collision) || collision_flags.test(Collision::has_bottom_collision); }

} // namespace shape
