
#include "Collider.hpp"
#include "../graphics/FLColor.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include <ccmath/math/power/sqrt.hpp>

namespace shape {

Collider::Collider() {
	dimensions = sf::Vector2<float>{default_dim, default_dim};
	jumpbox.dimensions = sf::Vector2<float>(dimensions.x, default_jumpbox_height);
	hurtbox.dimensions = sf::Vector2<float>(dimensions.x, dimensions.y);
	sync_components();
}

Collider::Collider(sf::Vector2<float> dim, sf::Vector2<float> start_pos) : dimensions(dim) {
	bounding_box.dimensions = dim;
	jumpbox.dimensions = sf::Vector2<float>(dim.x, default_jumpbox_height);
	hurtbox.dimensions = sf::Vector2<float>(dim.x, dim.y);
	sync_components();
}

void Collider::sync_components() {
	bounding_box.set_position(physics.position);
	vicinity.dimensions.x = dimensions.x + 2 * vicinity_pad;
	vicinity.dimensions.y = dimensions.y + 2 * vicinity_pad;
	wallslider.dimensions.x = dimensions.x + 2 * wallslide_pad;
	wallslider.dimensions.y = dimensions.y * 0.5f;
	predictive_vertical.dimensions.x = dimensions.x - 2 * vertical_detector_buffer;
	predictive_vertical.dimensions.y = dimensions.y + 2 * vertical_detector_buffer;
	predictive_horizontal.dimensions.x = dimensions.x + 2 * horizontal_detector_buffer;
	predictive_horizontal.dimensions.y = dimensions.y - 3 * horizontal_detector_buffer;
	predictive_combined.dimensions = dimensions;
	vicinity.set_position(sf::Vector2<float>{physics.position.x - vicinity_pad + physics.velocity.x, physics.position.y - vicinity_pad + physics.velocity.y});
	wallslider.set_position(sf::Vector2<float>{physics.position.x - wallslide_pad, physics.position.y + 8.f});
	predictive_vertical.set_position(sf::Vector2<float>{physics.position.x + vertical_detector_buffer, physics.position.y - vertical_detector_buffer + physics.velocity.y});
	predictive_horizontal.set_position(sf::Vector2<float>{physics.position.x - horizontal_detector_buffer + physics.velocity.x, physics.position.y + horizontal_detector_buffer});
	predictive_combined.set_position(sf::Vector2<float>{physics.position.x + physics.velocity.x, physics.position.y + physics.velocity.y});
	jumpbox.set_position(sf::Vector2<float>{physics.position.x, physics.position.y + dimensions.y});
	hurtbox.set_position(sf::Vector2<float>(physics.position.x + (dimensions.x / 2) - (hurtbox.dimensions.x / 2), physics.position.y + (dimensions.y / 2) - (hurtbox.dimensions.y / 2)));

	draw_hurtbox.setSize(hurtbox.dimensions);
	draw_hurtbox.setPosition(hurtbox.position);
}

void Collider::handle_map_collision(Shape const& cell, world::TileType tile_type) {

	flags.collision = {};

	// tile flags
	bool is_ground_ramp = tile_type == world::TileType::ground_ramp;
	bool is_ceiling_ramp = tile_type == world::TileType::ceiling_ramp;
	bool is_plat = tile_type == world::TileType::platform && (jumpbox.position.y > cell.position.y + 4 || physics.acceleration.y < 0.0f);
	bool is_spike = tile_type == world::TileType::spike;
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
	mtvs.combined = predictive_combined.testCollisionGetMTV(predictive_combined, cell);
	mtvs.vertical = predictive_vertical.testCollisionGetMTV(predictive_vertical, cell);
	mtvs.horizontal = predictive_horizontal.testCollisionGetMTV(predictive_horizontal, cell);
	mtvs.actual = bounding_box.testCollisionGetMTV(bounding_box, cell);

	float vert_threshold = 5.5f; // for landing
	// let's first settle all actual block collisions
	if (!is_ramp) {
		bool corner_collision{true};
		if (predictive_vertical.overlaps(cell)) {
			mtvs.vertical.y < 0.f ? flags.collision.set(Collision::has_bottom_collision) : flags.collision.set(Collision::has_top_collision);
			if (flags.collision.test(Collision::has_bottom_collision) && physics.velocity.y > vert_threshold) {
				flags.state.set(State::just_landed);
				flags.animation.set(Animation::just_landed);
			} // for landing sound
			corner_collision = false;
			correct_y(mtvs.vertical);
		}
		if (predictive_horizontal.overlaps(cell)) {
			mtvs.horizontal.x > 0.f ? flags.collision.set(Collision::has_left_collision) : flags.collision.set(Collision::has_right_collision);
			corner_collision = false;
			flags.dash.set(Dash::dash_cancel_collision);
			correct_x(mtvs.horizontal);
		}
		if (predictive_combined.overlaps(cell) && corner_collision) {
			flags.collision.set(Collision::any_collision);
			flags.dash.set(Dash::dash_cancel_collision);
			correct_corner(mtvs.combined);
		}
	}

	// now let's settle ramp collisions. remember, the collider has already been resolved from any previous cell collision
	if (is_ramp) {
		bool falls_onto = is_ground_ramp && physics.velocity.y > vert_threshold;
		bool jumps_into = physics.velocity.y < vert_threshold;
		// ground ramp
		// only handle ramp collisions if the bounding_box is colliding with it
		if (bounding_box.SAT(cell)) {
			if (is_ground_ramp && !flags.general.test(General::ignore_resolution)) {
				if (mtvs.actual.y < 0.f) { physics.position.y += mtvs.actual.y; }
				// still zero this because of gravity
				if (!flags.movement.test(Movement::jumping)) { physics.zero_y(); }
			}
			if (is_ceiling_ramp) { correct_x_y(mtvs.actual); }
			// cancel dash
			flags.dash.set(Dash::dash_cancel_collision);
		}
		// we also need to check if the predictive bounding box is colliding a ramp, just to deal with falling/jumping onto and into ramps
		if (predictive_combined.SAT(cell)) {
			if (falls_onto) {
				correct_x_y(mtvs.combined);
				flags.state.set(State::just_landed);
				flags.animation.set(Animation::just_landed);
			}
			if (is_ceiling_ramp) {
				if (jumps_into) {
					if (flags.movement.test(Movement::dashing)) {
						correct_y(mtvs.combined);
					} else {
						flags.collision.set(Collision::any_collision);
						correct_y(mtvs.combined + sf::Vector2<float>{8.f, 8.f}); // to prevent player gliding down ceiling ramps
					}
				}
			}
		}
	}

	set_depths();

	if (wallslider.overlaps(cell)) { wallslider.vertices.at(0).x > cell.vertices.at(0).x ? flags.state.set(State::left_wallslide_collision) : flags.state.set(State::right_wallslide_collision); }

	// long-winded, but I want to reserve SAT for colliders that actually need it
	if (flags.general.test(General::complex)) {
		if (jumpbox.SAT(cell)) {
			flags.state.set(State::grounded);
			flags.state.set(State::world_grounded);
			flags.state.set(State::is_any_jump_collision);
			flags.external_state.set(ExternalState::grounded);
		} else {
			flags.state.reset(State::grounded);
			flags.state.reset(State::world_grounded);
		}
	} else {
		if (jumpbox.overlaps(cell)) {
			flags.state.set(State::grounded);
			flags.state.set(State::world_grounded);
			flags.state.set(State::is_any_jump_collision);
			flags.external_state.set(ExternalState::grounded);
		} else {
			flags.state.reset(State::grounded);
			flags.state.reset(State::world_grounded);
		}
	}

	flags.movement.reset(Movement::dashing);

	sync_components();
}

void Collider::detect_map_collision(world::Map& map) {
	flags.external_state.reset(ExternalState::grounded);
	for (auto& index : map.collidable_indeces) {
		auto& cell = map.layers.at(world::MIDDLEGROUND).grid.cells.at(index);
		cell.collision_check = false;
		if (!map.nearby(cell.bounding_box, bounding_box)) {
			continue;
		} else {
			// check vicinity so we can escape early
			if (vicinity.vertices.empty()) { return; }
			if (!vicinity.overlaps(cell.bounding_box)) {
				continue;
			} else {
				cell.collision_check = true;
				if (cell.value > 0) { handle_map_collision(cell.bounding_box, cell.type); }
			}
		}
	}
}

int Collider::detect_ledge_height(world::Map& map) {
	int ret{};
	auto total = map.layers.at(world::MIDDLEGROUND).grid.cells.size();
	for (int index = 0; index < total; ++index) {
		auto& cell = map.layers.at(world::MIDDLEGROUND).grid.cells.at(index);
		if (!map.nearby(cell.bounding_box, bounding_box)) {
			continue;
		} else {
			// check vicinity so we can escape early
			if (vicinity.vertices.empty()) { return ret; }
			if (!vicinity.overlaps(cell.bounding_box)) {
				continue;
			} else {
				// we're in the vicinity now, so we check the bottom left and right corners to find a potential ledge
				if (!cell.is_occupied()) {
					auto right = cell.bounding_box.contains_point(vicinity.vertices.at(2));
					auto left = cell.bounding_box.contains_point(vicinity.vertices.at(3));
					if (left) { flags.state.set(State::ledge_left); }
					if (right) { flags.state.set(State::ledge_right); }
						if (left || right) { // left ledge found
						bool found{};
						auto next_row = index + map.dimensions.x;
						while (!found) {
							if (map.layers.at(world::MIDDLEGROUND).grid.cells.size() <= next_row) { return map.dimensions.y; };
							if (map.layers.at(world::MIDDLEGROUND).grid.cells.at(next_row).is_occupied()) { found = true; }
							next_row += map.dimensions.x;
							++ret;
							if (ret > (int)map.dimensions.y) { return map.dimensions.y; }
						}
						return ret;
					}
				}
			}
		}
	}
	return ret;
}

void Collider::correct_x(sf::Vector2<float> mtv) {
	if (flags.general.test(General::ignore_resolution)) { return; }
	auto xdist = predictive_horizontal.position.x + horizontal_detector_buffer - physics.position.x;
	auto correction = xdist + mtv.x;
	physics.position.x += correction;
	physics.zero_x();
}

void Collider::correct_y(sf::Vector2<float> mtv) {
	if (flags.general.test(General::ignore_resolution)) { return; }
	auto ydist = predictive_vertical.position.y + vertical_detector_buffer - physics.position.y;
	auto correction = ydist + mtv.y;
	physics.position.y += correction;
	physics.zero_y();
}

void Collider::correct_x_y(sf::Vector2<float> mtv) {
	if (flags.general.test(General::ignore_resolution)) { return; }
	auto xdist = predictive_combined.position.x - physics.position.x;
	auto correction = xdist + mtv.x;
	physics.position.x += correction;
	physics.zero_x();
	auto ydist = predictive_combined.position.y - physics.position.y;
	correction = ydist + mtv.y;
	physics.position.y += correction;
	physics.zero_y();
}

void Collider::correct_corner(sf::Vector2<float> mtv) {
	if (flags.general.test(General::ignore_resolution)) { return; }
	auto ydist = predictive_vertical.position.y - physics.position.y;
	auto correction = ydist + mtv.y;
	physics.position.y += correction;
	physics.zero_y();
}

void Collider::set_depths() {
	if (flags.collision.test(Collision::has_top_collision)) { collision_depths.top = std::max(collision_depths.top, abs(mtvs.vertical.y)); }
	if (flags.collision.test(Collision::has_bottom_collision)) { collision_depths.bottom = std::max(collision_depths.bottom, abs(mtvs.vertical.y)); }
	if (flags.collision.test(Collision::has_left_collision)) { collision_depths.left = std::max(collision_depths.left, abs(mtvs.horizontal.x)); }
	if (flags.collision.test(Collision::has_right_collision)) { collision_depths.right = std::max(collision_depths.right, abs(mtvs.horizontal.x)); }
}

void Collider::handle_platform_collision(Shape const& cell) {}

void Collider::handle_spike_collision(Shape const& cell) {
	if (hurtbox.overlaps(cell)) { spike_trigger = true; }
}

void Collider::handle_collider_collision(Shape const& collider) {
	if (flags.general.test(General::ignore_resolution)) { return; }

	flags.collision = {};

	// store all four mtvs
	mtvs.combined = predictive_combined.testCollisionGetMTV(predictive_combined, collider);
	mtvs.vertical = predictive_vertical.testCollisionGetMTV(predictive_vertical, collider);
	mtvs.horizontal = predictive_horizontal.testCollisionGetMTV(predictive_horizontal, collider);
	mtvs.actual = bounding_box.testCollisionGetMTV(bounding_box, collider);

	float vert_threshold = 5.5f;
	bool corner_collision{true};
	if (predictive_vertical.overlaps(collider)) {
		mtvs.vertical.y < 0.f ? flags.collision.set(Collision::has_bottom_collision) : flags.collision.set(Collision::has_top_collision);
		if (flags.collision.test(Collision::has_bottom_collision) && physics.velocity.y > vert_threshold) {
			flags.state.set(State::just_landed);
			flags.animation.set(Animation::just_landed);
		} // for landing sound
		corner_collision = false;
		correct_y(mtvs.vertical);
	}
	if (predictive_horizontal.overlaps(collider)) {
		mtvs.horizontal.x > 0.f ? flags.collision.set(Collision::has_left_collision) : flags.collision.set(Collision::has_right_collision);
		corner_collision = false;
		flags.dash.set(Dash::dash_cancel_collision);
		correct_x(mtvs.horizontal);
	}
	if (predictive_combined.overlaps(collider) && corner_collision) {
		flags.collision.set(Collision::any_collision);
		flags.dash.set(Dash::dash_cancel_collision);
		correct_corner(mtvs.combined);
	}
	if (jumpbox.SAT(collider)) {
		flags.state.set(State::grounded);
		flags.state.set(State::is_any_jump_collision);
	} else {
		flags.state.reset(State::grounded);
	}

	set_depths();

	flags.movement.reset(Movement::dashing);

	sync_components();
}

void Collider::update(automa::ServiceProvider& svc) {
	physics.update(svc);
	sync_components();
	flags.state.reset(State::just_collided);
	physics.gravity = flags.state.test(State::grounded) ? 0.0f : stats.GRAV;
	flags.state.test(State::grounded) ? physics.flags.set(components::State::grounded) : physics.flags.reset(components::State::grounded);
}

void Collider::render(sf::RenderWindow& win, sf::Vector2<float> cam) {

	// draw predictive vertical
	box.setSize(predictive_vertical.dimensions);
	box.setPosition(predictive_vertical.position.x - cam.x, predictive_vertical.position.y - cam.y);
	box.setOutlineColor(sf::Color{255, 0, 0, 120});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	//win.draw(box);

	// draw predictive horizontal
	box.setSize(predictive_horizontal.dimensions);
	box.setPosition(predictive_horizontal.position.x - cam.x, predictive_horizontal.position.y - cam.y);
	box.setOutlineColor(sf::Color{0, 0, 255, 120});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	//win.draw(box);

	// draw predictive combined
	box.setSize(predictive_combined.dimensions);
	box.setPosition(predictive_combined.position.x - cam.x, predictive_combined.position.y - cam.y);
	box.setOutlineColor(sf::Color{255, 255, 255, 120});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	//win.draw(box);

	// draw bounding box
	box.setSize(dimensions);
	box.setPosition(bounding_box.position.x - cam.x, bounding_box.position.y - cam.y);
	box.setFillColor(sf::Color{200, 150, 255, 80});
	box.setOutlineColor(sf::Color{255, 255, 255, 255});
	box.setOutlineThickness(-1);
	win.draw(box);

	// draw jump box
	box.setSize(jumpbox.dimensions);
	box.setPosition(jumpbox.position.x - cam.x, jumpbox.position.y - cam.y);
	box.setFillColor(flcolor::navy_blue);
	box.setOutlineColor(sf::Color::Transparent);
	flags.state.test(State::is_any_jump_collision) ? box.setFillColor(flcolor::blue) : box.setFillColor(flcolor::navy_blue);
	win.draw(box);

	// draw hurtbox
	draw_hurtbox.setSize(sf::Vector2<float>{(float)hurtbox.dimensions.x, (float)hurtbox.dimensions.y});
	draw_hurtbox.setPosition(hurtbox.position.x - cam.x, hurtbox.position.y - cam.y);
	win.draw(draw_hurtbox);

	// draw vicinity
	box.setSize(sf::Vector2<float>{(float)vicinity.dimensions.x, (float)vicinity.dimensions.y});
	box.setPosition(vicinity.position.x - cam.x, vicinity.position.y - cam.y);
	box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(sf::Color{120, 60, 80, 180});
	box.setOutlineThickness(-1);
	win.draw(box);

	// draw wallslider
	box.setSize(sf::Vector2<float>{(float)wallslider.dimensions.x, (float)wallslider.dimensions.y});
	box.setPosition(wallslider.position.x - cam.x, wallslider.position.y - cam.y);
	has_left_wallslide_collision() || has_right_wallslide_collision() ? box.setFillColor(sf::Color::Blue) : box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(sf::Color{60, 60, 180, 100});
	box.setOutlineThickness(-1);
	//win.draw(box);

	// draw physics position
	box.setSize({2.f, 2.f});
	box.setPosition(physics.position.x - cam.x, physics.position.y - cam.y);
	box.setFillColor(flcolor::green);
	box.setOutlineThickness(0);
	//win.draw(box);
}
void Collider::reset() { flags.state = {}; }
void Collider::reset_ground_flags() {

	if (flags.state.test(State::is_any_jump_collision)) {
		flags.state.set(State::grounded);
	} else {
		flags.state.reset(State::grounded);
	}
}
bool Collider::on_ramp() { return flags.state.test(State::on_ramp); }

bool Collider::has_horizontal_collision() const { return flags.collision.test(Collision::has_left_collision) || flags.collision.test(Collision::has_right_collision); }

bool Collider::has_left_collision() const { return flags.collision.test(Collision::has_left_collision); }

bool Collider::has_right_collision() const { return flags.collision.test(Collision::has_right_collision); }

bool Collider::has_vertical_collision() const { return flags.collision.test(Collision::has_top_collision) || flags.collision.test(Collision::has_bottom_collision); }

bool Collider::has_left_wallslide_collision() const { return flags.state.test(State::left_wallslide_collision); }

bool Collider::has_right_wallslide_collision() const { return flags.state.test(State::right_wallslide_collision); }

float Collider::compute_length(sf::Vector2<float> const v) { return ccm::sqrt(v.x * v.x + v.y * v.y); }

} // namespace shape
