
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
	jumpbox.dimensions = sf::Vector2<float>(dimensions.x, default_jumpbox_height);
	hurtbox.dimensions = sf::Vector2<float>(dimensions.x / 2, dimensions.y / 2);
}

Collider::Collider(sf::Vector2<float> dim, sf::Vector2<float> start_pos) : dimensions(dim) {

	physics = components::PhysicsComponent({0.9, 0.9}, 1);

	bounding_box.dimensions = dim;
	predictive_vertical.dimensions = dim;
	predictive_horizontal.dimensions = dim;
	predictive_combined.dimensions = dim;
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

	detector_buffer = 2.f;
	float vertical_detector_buffer{1.f};

	bounding_box.set_position(physics.position);
	predictive_vertical.dimensions.x = dimensions.x - 2 * vertical_detector_buffer;
	predictive_horizontal.dimensions.x = dimensions.x + 2 * detector_buffer;
	predictive_horizontal.dimensions.y = dimensions.y - 3 * detector_buffer;
	predictive_combined.dimensions = dimensions;
	predictive_vertical.set_position(sf::Vector2<float>{physics.position.x + vertical_detector_buffer, physics.position.y + physics.velocity.y});
	predictive_horizontal.set_position(sf::Vector2<float>{physics.position.x - detector_buffer + physics.velocity.x, physics.position.y + detector_buffer});
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

	//special tile types
	if (is_plat) {
		handle_platform_collision(cell);
		return;
	}
	if (is_spike) {
		handle_spike_collision(cell);
		return;
	}

	// stanard vertical correction
	if (predictive_vertical.SAT(cell)) {
		collision_flags.set(Collision::any_collision);
		// set mtv
		physics.mtv = predictive_vertical.testCollisionGetMTV(predictive_vertical, cell);
		if (!is_ramp && physics.velocity.y < 0.f) {
			physics.acceleration.y = 0.f;
			physics.velocity.y = 0.f;
			physics.position.y += physics.mtv.y;
		} else {
			if (physics.velocity.y > landed_threshold) { flags.set(State::just_landed); }
			correct_y();
		}
		physics.mtv.y < 0.f ? collision_flags.set(Collision::has_bottom_collision) : collision_flags.set(Collision::has_top_collision);
		physics.mtv = {};
	}

	//ramp correction
	if (is_ramp) {

		bool falls_onto_ramp = is_ground_ramp && physics.velocity.y > 4.1f;
		bool walks_up_ramp = is_ground_ramp;
		bool jumps_into_ramp = is_ceiling_ramp && physics.velocity.y < 4.1f;

		if (bounding_box.SAT(cell)) {
			collision_flags.set(Collision::ramp_collision);
			physics.mtv = bounding_box.testCollisionGetMTV(bounding_box, cell);
			if (falls_onto_ramp) {
				if (physics.velocity.y > landed_threshold) { flags.set(State::just_landed); }
				auto temp_mtv = bounding_box.testCollisionGetMTV(bounding_box, cell);
				bool one_overlaps = !(abs(temp_mtv.x) > 0.f) && !(abs(temp_mtv.y) > 0.f);

				// case 1, where the bounding box has some distance from the ramp
				if (one_overlaps) {
					auto xdist = predictive_combined.position.x - physics.position.x;
					auto correction = xdist - physics.mtv.x;

					physics.position.x += correction;

					auto ydist = predictive_combined.position.y - physics.position.y;
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
			if (walks_up_ramp && !collision_flags.test(Collision::has_top_collision)) {
				physics.position.y += physics.mtv.y;
				// still zero this because of gravity
				physics.velocity.y = 0.0f;
				physics.acceleration.y = 0.0f;
				//if the collider is dashing
				if (abs(physics.velocity.x) > physics.maximum_velocity.x * 0.5) { jumps_into_ramp = true; }
			}
			if (jumps_into_ramp) {
				auto temp_mtv = bounding_box.testCollisionGetMTV(bounding_box, cell);
				bool one_overlaps = !(abs(temp_mtv.x) > 0.f) && !(abs(temp_mtv.y) > 0.f);

				// case 1, where the bounding box has some distance from the ramp
				if (one_overlaps) {
					auto xdist = physics.position.x - predictive_combined.position.x;
					auto correction = xdist - physics.mtv.x;

					physics.position.x -= correction * 0.9f;

					auto ydist = physics.position.y - predictive_combined.position.y;
					correction = ydist - physics.mtv.y;
					physics.position.y -= correction;
					physics.velocity.y = 0.0f;
					physics.acceleration.y = 0.0f;

					// case 2, where both the bounding box and the predictive bounding box overlap the ramp
				} else {
					physics.position.x += temp_mtv.x;
					physics.position.y += temp_mtv.y;
				}
			}
		}
	}
	// standard horizontal correction
	if (predictive_horizontal.SAT(cell)) {
		collision_flags.set(Collision::any_collision);
		// set mtv
		physics.mtv = predictive_horizontal.testCollisionGetMTV(predictive_horizontal, cell);
		physics.mtv.x > 0.f ? collision_flags.set(Collision::has_left_collision) : collision_flags.set(Collision::has_right_collision);
		if (!is_ramp) { correct_x(); }
		physics.mtv = {};
	}
	
	//final check if above corrections yielded unusual results
	if (predictive_combined.SAT(cell)) {
		physics.mtv = bounding_box.testCollisionGetMTV(bounding_box, cell);
		physics.position += physics.mtv;
	}

	if (jumpbox.SAT(cell)) {
		flags.set(State::grounded);
		flags.set(State::is_any_jump_collision);
	} else {
		flags.reset(State::grounded);
	}

	//don't slide down ramps
	if (flags.test(State::grounded) && physics.velocity.y > 0.f) {
		physics.acceleration.y = 0;
		physics.velocity.y = 0.f;
	}

	sync_components();
}

void Collider::correct_x() {
	auto xdist = predictive_horizontal.position.x + detector_buffer - physics.position.x;
	auto correction = xdist + physics.mtv.x;
	physics.position.x += correction;
	physics.acceleration.x = 0.0f;
	physics.velocity.x = 0.0f;
}

void Collider::correct_y() {
	auto ydist = predictive_vertical.position.y - physics.position.y;
	auto correction = ydist + physics.mtv.y;
	physics.position.y += correction;
	physics.acceleration.y = 0.0f;
	physics.velocity.y = 0.0f;
}

void Collider::handle_platform_collision(Shape const& cell) {}

void Collider::handle_spike_collision(Shape const& cell) {
	if (hurtbox.SAT(cell)) { spike_trigger = true; }
}

void Collider::handle_collider_collision(Shape const& collider) {}

void Collider::update() {
	// if (!flags.test(State::is_colliding_with_level)) { physics.mtv = {0.0f, 0.0f}; }
	flags.reset(State::just_collided);
	physics.gravity = flags.test(State::grounded) ? 0.0f : stats.GRAV;
	flags.test(State::grounded) ? physics.flags.set(components::State::grounded) : physics.flags.reset(components::State::grounded);
}

void Collider::render(sf::RenderWindow& win, sf::Vector2<float> cam) {

	// draw predictive vertical
	box.setSize(predictive_vertical.dimensions);
	box.setPosition(predictive_vertical.position.x - cam.x, predictive_vertical.position.y - cam.y);
	box.setOutlineColor(flcolor::fucshia);
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw predictive horizontal
	box.setSize(predictive_horizontal.dimensions);
	box.setPosition(predictive_horizontal.position.x - cam.x, predictive_horizontal.position.y - cam.y);
	box.setOutlineColor(flcolor::goldenrod);
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw predictive combined
	box.setSize(predictive_combined.dimensions);
	box.setPosition(predictive_combined.position.x - cam.x, predictive_combined.position.y - cam.y);
	box.setOutlineColor(flcolor::green);
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw bounding box
	box.setSize(dimensions);
	box.setPosition(bounding_box.position.x - cam.x, bounding_box.position.y - cam.y);
	box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(sf::Color{255, 255, 255, 80});
	box.setOutlineThickness(-1);
	// flags.test(State::is_colliding_with_level) ? box.setFillColor(sf::Color{90, 100, 20, 60}) : box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw jump box
	box.setSize(jumpbox.dimensions);
	box.setPosition(jumpbox.position.x - cam.x, jumpbox.position.y - cam.y);
	box.setFillColor(flcolor::blue);
	box.setOutlineColor(sf::Color::Transparent);
	flags.test(State::is_any_jump_collision) ? box.setFillColor(flcolor::ui_white) : box.setFillColor(flcolor::blue);
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

bool Collider::has_horizontal_collision() const { return collision_flags.test(Collision::has_left_collision) || collision_flags.test(Collision::has_right_collision); }

bool Collider::has_left_collision() const { return collision_flags.test(Collision::has_left_collision); }

bool Collider::has_right_collision() const { return collision_flags.test(Collision::has_right_collision); }

bool Collider::has_vertical_collision() const { return collision_flags.test(Collision::has_top_collision) || collision_flags.test(Collision::has_bottom_collision); }

} // namespace shape
