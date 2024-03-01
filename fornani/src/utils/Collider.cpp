
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
	predictive_vertical.dimensions = dim;
	predictive_horizontal.dimensions = dim;
	jumpbox.dimensions = sf::Vector2<float>(dim.x, default_jumpbox_height);
	left_detector.dimensions = sf::Vector2<float>(default_detector_width, dim.y - 1);
	right_detector.dimensions = sf::Vector2<float>(default_detector_width, dim.y - 1);
	top_detector.dimensions = sf::Vector2<float>(dim.x, default_detector_width);
	bottom_detector.dimensions = sf::Vector2<float>(dim.x, default_detector_width);
	hurtbox.dimensions = sf::Vector2<float>(dim.x / 2, dim.y / 2);

	detector_buffer = (dim.y - left_detector.dimensions.y) / 2;

	bounding_box.position = start_pos;
	predictive_vertical.position = start_pos;
	predictive_horizontal.position = start_pos;
	jumpbox.position = sf::Vector2<float>(start_pos.x, start_pos.y + dim.y);
	left_detector.position = sf::Vector2<float>(start_pos.x - left_detector.dimensions.x, start_pos.y + detector_buffer);
	right_detector.position = sf::Vector2<float>(start_pos.x + dim.x, start_pos.y + detector_buffer);
	top_detector.position = sf::Vector2<float>(start_pos.x, start_pos.y - top_detector.dimensions.y);
	bottom_detector.position = sf::Vector2<float>(start_pos.x, start_pos.y + dim.y - bottom_detector.dimensions.y);

	hurtbox.position = sf::Vector2<float>(start_pos.x + (dim.x / 2) - (hurtbox.dimensions.x / 2), start_pos.y + (dim.y / 2) - (hurtbox.dimensions.y / 2));
}

void Collider::sync_components() {

	detector_buffer = 2.f;

	bounding_box.set_position(physics.position);
	predictive_vertical.dimensions.x = dimensions.x - 2 * detector_buffer;
	predictive_horizontal.dimensions.y = dimensions.y - 2 * detector_buffer;
	predictive_vertical.set_position(sf::Vector2<float>{physics.position.x + detector_buffer, physics.position.y + physics.velocity.y});
	predictive_horizontal.set_position(sf::Vector2<float>{physics.position.x + physics.velocity.x, physics.position.y + detector_buffer});
	jumpbox.set_position(sf::Vector2<float>{physics.position.x, physics.position.y + dimensions.y});
	left_detector.set_position(sf::Vector2<float>{physics.position.x - detector_buffer, physics.position.y});
	right_detector.set_position(sf::Vector2<float>{physics.position.x + dimensions.x - default_detector_width + detector_buffer, physics.position.y});
	hurtbox.set_position(sf::Vector2<float>(physics.position.x + (dimensions.x / 2) - (hurtbox.dimensions.x / 2), physics.position.y + (dimensions.y / 2) - (hurtbox.dimensions.y / 2)));
	top_detector.set_position(sf::Vector2<float>(physics.position.x, physics.position.y - detector_buffer));
	bottom_detector.set_position(sf::Vector2<float>(physics.position.x, physics.position.y + dimensions.y - bottom_detector.dimensions.y));
}

void Collider::handle_map_collision(Shape const& cell, lookup::TILE_TYPE tile_type) {

	collision_flags = {};

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
	// if (higher_than_cell_top) {
	float y_dist = cell.vertices[0].y - left_detector.vertices[2].y;
	bool is_ramp = (is_ground_ramp || is_ceiling_ramp);

	// standard horizontal correction
	if (predictive_horizontal.SAT(cell)) {
		// set mtv
		physics.mtv = predictive_horizontal.testCollisionGetMTV(predictive_horizontal, cell);
		if (!is_plat && !is_spike && !is_ramp) { correct_x(); }
		physics.mtv.x > 0.f ? collision_flags.set(Collision::has_left_collision) : collision_flags.set(Collision::has_right_collision);
		physics.mtv = {};
	}

	// stanard vertical correction
	if (predictive_vertical.SAT(cell)) {
		// set mtv
		physics.mtv = predictive_vertical.testCollisionGetMTV(predictive_vertical, cell);
		if (!is_plat && !is_spike && !is_ramp) { correct_y(); }
		physics.mtv.y < 0.f ? collision_flags.set(Collision::has_bottom_collision) : collision_flags.set(Collision::has_top_collision);
		physics.mtv = {};
	}

	if (is_ramp) {

		bool falls_onto_ramp = is_ground_ramp && physics.velocity.y > 4.1f;
		bool walks_up_ramp = is_ground_ramp;
		bool jumps_into_ramp = is_ceiling_ramp && physics.velocity.y < 4.1f;

		if (predictive_horizontal.SAT(cell)) {
			physics.mtv = predictive_horizontal.testCollisionGetMTV(predictive_horizontal, cell);
			if (walks_up_ramp && !collision_flags.test(Collision::has_top_collision)) {
				physics.position.y += physics.mtv.y;
				// still zero this because of gravity
				physics.velocity.y = 0.0f;
				physics.acceleration.y = 0.0f;
			}
		}
		if (predictive_vertical.SAT(cell)) {
			physics.mtv = predictive_vertical.testCollisionGetMTV(predictive_vertical, cell);
			if (falls_onto_ramp || jumps_into_ramp) {
				correct_y();
				correct_x();
			}
		}
	}

	if (jumpbox.SAT(cell) && !is_spike) {
		flags.set(State::grounded);
		flags.set(State::is_any_jump_collision);
	} else {
		flags.reset(State::grounded);
	}

	sync_components();
}

void Collider::correct_x() {
	auto xdist = predictive_horizontal.position.x - physics.position.x;
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

	// draw bounding box
	box.setSize(dimensions);
	box.setPosition(bounding_box.position.x - cam.x, bounding_box.position.y - cam.y);
	box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(sf::Color{255, 255, 255, 80});
	box.setOutlineThickness(-1);
	// flags.test(State::is_colliding_with_level) ? box.setFillColor(sf::Color{90, 100, 20, 60}) : box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw left detector
	box.setSize(sf::Vector2<float>{(float)left_detector.dimensions.x, (float)left_detector.dimensions.y});
	box.setPosition(left_detector.position.x - cam.x, left_detector.position.y - cam.y);
	box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(flcolor::periwinkle);
	// flags.test(State::has_left_collision) ? box.setFillColor(flcolor::goldenrod) : box.setFillColor(sf::Color{50, 60, 130, 80});
	//  win.draw(box);

	// draw right detector
	box.setSize(sf::Vector2<float>{(float)right_detector.dimensions.x, (float)right_detector.dimensions.y});
	box.setPosition(right_detector.position.x - cam.x, right_detector.position.y - cam.y);
	box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(flcolor::orange);
	// flags.test(State::has_right_collision) ? box.setFillColor(flcolor::goldenrod) : box.setFillColor(sf::Color{50, 60, 130, 80});
	//  win.draw(box);

	// draw top detector
	box.setSize(sf::Vector2<float>{(float)top_detector.dimensions.x, (float)top_detector.dimensions.y});
	box.setPosition(top_detector.position.x - cam.x, top_detector.position.y - cam.y);
	// flags.test(State::has_top_collision) ? box.setFillColor(flcolor::goldenrod) : box.setFillColor(sf::Color{50, 60, 130, 80});
	//  win.draw(box);

	// draw bottom detector
	box.setSize(sf::Vector2<float>{(float)bottom_detector.dimensions.x, (float)bottom_detector.dimensions.y});
	box.setPosition(bottom_detector.position.x - cam.x, bottom_detector.position.y - cam.y);
	// flags.test(State::has_bottom_collision) ? box.setFillColor(flcolor::goldenrod) : box.setFillColor(sf::Color{50, 60, 130, 80});
	//  win.draw(box);

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
