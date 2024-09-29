
#include "Collider.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include <ccmath/math/power/sqrt.hpp>

namespace shape {

Collider::Collider() {
	dimensions = sf::Vector2<float>{default_dim, default_dim};
	jumpbox.dimensions = sf::Vector2<float>(dimensions.x, default_jumpbox_height);
	hurtbox.dimensions = sf::Vector2<float>(dimensions.x - 8.f, dimensions.y - 8.f);
	sync_components();
}

Collider::Collider(sf::Vector2<float> dim, sf::Vector2<float> start_pos, sf::Vector2<float> hbx_offset) : dimensions(dim), hurtbox_offset(hbx_offset) {
	bounding_box.dimensions = dim;
	jumpbox.dimensions = sf::Vector2<float>(dim.x, default_jumpbox_height);
	hurtbox.dimensions = sf::Vector2<float>(dim.x - 8.f, dim.y - 8.f + hurtbox_offset.y);
	sync_components();
}

void Collider::sync_components() {
	bounding_box.set_position(physics.position);
	bounding_box.dimensions = dimensions;
	vicinity.dimensions.x = dimensions.x + 2.f * vicinity_pad;
	vicinity.dimensions.y = dimensions.y + 2.f * vicinity_pad;
	wallslider.dimensions.x = dimensions.x + 2.f * wallslide_pad;
	wallslider.dimensions.y = dimensions.y * 0.7f;

	vertical.dimensions.x = 1.f;
	vertical.dimensions.y = dimensions.y - 2.f * depth_buffer;
	horizontal.dimensions.x = dimensions.x - 2.f * depth_buffer;
	horizontal.dimensions.y = 1.f;

	predictive_vertical.dimensions.x = dimensions.x - 2.f * vertical_detector_buffer;
	predictive_vertical.dimensions.y = dimensions.y + 2.f * vertical_detector_buffer;
	predictive_horizontal.dimensions.x = dimensions.x + 2.f * horizontal_detector_buffer;
	predictive_horizontal.dimensions.y = dimensions.y - 3.f * horizontal_detector_buffer;
	predictive_combined.dimensions = dimensions;

	vicinity.set_position(sf::Vector2<float>{physics.position.x - vicinity_pad + physics.velocity.x, physics.position.y - vicinity_pad + physics.velocity.y});
	wallslider.set_position(sf::Vector2<float>{physics.position.x - wallslide_pad, physics.position.y + 2.f});
	predictive_vertical.set_position(sf::Vector2<float>{physics.position.x + vertical_detector_buffer, physics.position.y - vertical_detector_buffer + physics.velocity.y});
	predictive_horizontal.set_position(sf::Vector2<float>{physics.position.x - horizontal_detector_buffer + physics.velocity.x, physics.position.y + horizontal_detector_buffer});
	predictive_combined.set_position(sf::Vector2<float>{physics.position.x + physics.velocity.x, physics.position.y + physics.velocity.y});
	jumpbox.set_position(sf::Vector2<float>{physics.position.x, physics.position.y + dimensions.y});
	hurtbox.set_position(sf::Vector2<float>(physics.position.x + (dimensions.x * 0.5f) - (hurtbox.dimensions.x * 0.5f), physics.position.y + (dimensions.y * 0.5f) - (hurtbox.dimensions.y * 0.5f) - (hurtbox_offset.y * 0.5f)));
	vertical.set_position(sf::Vector2<float>{physics.position.x + dimensions.x * 0.5f - 0.5f, physics.position.y + depth_buffer});
	horizontal.set_position(sf::Vector2<float>{physics.position.x + depth_buffer, physics.position.y + dimensions.y * 0.5f - 0.5f});

	draw_hurtbox.setFillColor(sf::Color::Transparent);
	draw_hurtbox.setOutlineColor(sf::Color::Blue);
	draw_hurtbox.setOutlineThickness(-1);
	draw_hurtbox.setSize(hurtbox.dimensions);
	draw_hurtbox.setPosition(hurtbox.position);
}

void Collider::handle_map_collision(world::Tile const& tile) {
	if (collision_depths) {
		if (collision_depths.value().crushed()) { return; }
	}

	flags.collision = {};
	auto const& cell = tile.bounding_box;

	// tile flags
	bool is_ground_ramp = tile.is_ground_ramp();
	bool is_ceiling_ramp = tile.is_ceiling_ramp();
	bool is_plat = tile.is_platform() && (jumpbox.position.y > cell.position.y + 4 || physics.acceleration.y < 0.0f);
	bool is_spike = tile.is_spike();
	bool is_ramp = tile.is_ramp();

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
		if (collision_depths) { collision_depths.value().calculate(*this, cell); }
		bool corner_collision{true};
		if (predictive_vertical.SAT(cell)) {
			mtvs.vertical.y < 0.f ? flags.collision.set(Collision::has_bottom_collision) : flags.collision.set(Collision::has_top_collision);
			//if (abs(mtvs.combined.x > 0.0001f)) { std::cout << "Combined MTV reading: " << mtvs.combined.x << "\n"; }
			if (flags.collision.test(Collision::has_bottom_collision) && physics.velocity.y > vert_threshold) {
				flags.state.set(State::just_landed);
				flags.animation.set(Animation::just_landed);
			} // for landing sound
			corner_collision = false;
			flags.external_state.set(ExternalState::world_collision);
			flags.external_state.set(ExternalState::vert_world_collision);
			correct_y(mtvs.vertical);
		}
		if (predictive_horizontal.SAT(cell) && !tile.ramp_adjacent()) {
			mtvs.horizontal.x > 0.f ? flags.collision.set(Collision::has_left_collision) : flags.collision.set(Collision::has_right_collision);
			corner_collision = false;
			flags.dash.set(Dash::dash_cancel_collision);
			flags.external_state.set(ExternalState::world_collision);
			flags.external_state.set(ExternalState::horiz_world_collision);
			flags.external_state.reset(ExternalState::vert_world_collision);
			correct_x(mtvs.horizontal);
		}
		if (predictive_combined.SAT(cell) && corner_collision && !tile.ramp_adjacent()) {
			//if (abs(mtvs.combined.x > 0.0001f)) { std::cout << "Combined MTV reading: " << mtvs.combined.x << "\n"; }
			flags.collision.set(Collision::any_collision);
			flags.dash.set(Dash::dash_cancel_collision);
			flags.external_state.set(ExternalState::world_collision);

			correct_corner(mtvs.combined);
		}
		if (flags.external_state.test(ExternalState::vert_world_collision)) { flags.external_state.reset(ExternalState::horiz_world_collision); }
	}
	// don't fix ramp position unless we're only colliding with ramps
	if (!is_ramp && jumpbox.SAT(cell)) { flags.state.set(State::on_flat_surface); }
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
				// std::cout << "\nGround ramp collision with MTV y of: " << mtvs.actual.y;
				if (mtvs.actual.y > 4.f) { physics.position.y -= mtvs.actual.y; } // player gets stuck in a thin ramp
			}
			if (is_ceiling_ramp) { correct_x_y(mtvs.actual); }
			// cancel dash
			flags.dash.set(Dash::dash_cancel_collision);
			flags.external_state.set(ExternalState::world_collision);
		}
		// we also need to check if the predictive bounding box is colliding a ramp, just to deal with falling/jumping onto and into ramps
		if (predictive_combined.SAT(cell)) {
			if (falls_onto) {
				correct_corner(mtvs.combined);
				flags.state.set(State::just_landed);
				flags.animation.set(Animation::just_landed);
				flags.external_state.set(ExternalState::world_collision);
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
				flags.external_state.set(ExternalState::world_collision);
			}
		}
		if (jumpbox.SAT(cell) && !flags.state.test(State::on_flat_surface) && !flags.movement.test(Movement::jumping)) {
			if (tile.is_negative_ramp()) { maximum_ramp_height = std::max(maximum_ramp_height, cell.get_height_at(abs(physics.position.x - cell.position.x))); }
			if (tile.is_positive_ramp()) { maximum_ramp_height = std::max(maximum_ramp_height, cell.get_height_at(abs(physics.position.x + dimensions.x - cell.position.x))); }
			physics.position.y = cell.position.y + cell.dimensions.y - maximum_ramp_height - dimensions.y;
			if ((physics.velocity.x > 0.f && tile.is_negative_ramp()) || (physics.velocity.x < 0.f && tile.is_positive_ramp())) { flags.perma_state.set(PermaFlags::downhill); }
		} else if (flags.state.test(State::on_flat_surface)) {
			flags.perma_state.reset(PermaFlags::downhill);
		}
	}
	if (!is_ramp && wallslider.overlaps(cell)) { wallslider.vertices.at(0).x > cell.vertices.at(0).x ? flags.state.set(State::left_wallslide_collision) : flags.state.set(State::right_wallslide_collision); }

	// long-winded, but I want to reserve SAT for colliders that actually need it
	if (flags.general.test(General::complex)) {
		if (jumpbox.SAT(cell)) {
			flags.state.set(State::grounded);
			flags.state.set(State::world_grounded);
			flags.external_state.set(ExternalState::world_grounded);
			flags.perma_state.set(PermaFlags::world_grounded); 
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
			flags.external_state.set(ExternalState::world_grounded);
			flags.perma_state.set(PermaFlags::world_grounded); 
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
	flags.perma_state = {};

	auto& grid = map.get_layers().at(world::MIDDLEGROUND).grid;
	auto range = get_collision_range(map);

	for(auto i{range.first}; i < range.second; ++i) {
		auto& cell = grid.get_cell(i);
		if (!cell.is_collidable()) { continue; }
		if (!map.nearby(cell.bounding_box, bounding_box)) {
			continue;
		} else {
			// check vicinity so we can escape early
			if (vicinity.vertices.empty()) { return; }
			if (!vicinity.overlaps(cell.bounding_box)) {
				continue;
			} else {
				cell.collision_check = true;
				if (cell.value > 0) { handle_map_collision(cell); }
			}
		}
	}
	maximum_ramp_height = 0.f;
	flags.state.reset(State::on_flat_surface);
}

std::pair<size_t, size_t> Collider::get_collision_range(world::Map& map) {
	auto& grid = map.get_layers().at(world::MIDDLEGROUND).grid;
	auto start_index = sf::Vector2<size_t>(static_cast<size_t>((vicinity.vertices.at(0).x / 32)), static_cast<size_t>((vicinity.vertices.at(0).y / 32)));
	auto end_index = sf::Vector2<size_t>(static_cast<size_t>((vicinity.vertices.at(2).x / 32)), static_cast<size_t>((vicinity.vertices.at(2).y / 32)));
	auto start = static_cast<size_t>(map.dimensions.x) * start_index.y + start_index.x;
	auto end = static_cast<size_t>(map.dimensions.x) * end_index.y + end_index.x + 1;
	start = std::clamp(start, std::size_t{0}, static_cast<size_t>(grid.cells.size() - 1));
	end = std::clamp(end, std::size_t{0}, static_cast<size_t>(grid.cells.size() - 1));
	return {start, end};
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
	// for large mtv values, overcorrect to prevent clipping
	if (abs(mtv.x) > 12.f || abs(mtv.y) > 12.f) {
		mtv.x = abs(mtv.y) > 0 ? mtv.y : mtv.x;
		mtv.y = abs(mtv.x) > 0 ? mtv.x : mtv.y;
		//std::cout << "large MTV!\n";
	}
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
	if (abs(mtv.x) >= abs(mtv.y)) {
		physics.position.x = predictive_combined.position.x + mtv.x;
		physics.zero_x();
		//std::cout << "X Corner correction: " << mtv.x << "\n";
	} else {
		auto ydist = predictive_combined.position.y - physics.position.y;
		auto correction = ydist + mtv.y;
		physics.position.y = predictive_combined.position.y + mtv.y;
		physics.zero_y();
		//std::cout << "Y Corner correction: " << correction << "\n";
	}
}

void Collider::resolve_depths() {
	if (!collision_depths) { return; }
	auto downward = collision_depths.value().top_depth() > collision_depths.value().bottom_depth();
	auto rightward = collision_depths.value().left_depth() > collision_depths.value().right_depth();
	sf::Vector2<float> depth_diff{};
	depth_diff.y = (collision_depths.value().top_depth() + collision_depths.value().bottom_depth()) * 0.5f;
	depth_diff.x = (collision_depths.value().left_depth() + collision_depths.value().right_depth()) * 0.5f;
	if (depth_diff.x < 8.f && depth_diff.y < 8.f) { return; }
	depth_diff.y *= downward ? 1.f : -1.f;
	depth_diff.x *= rightward ? 1.f : -1.f;
	physics.position.x += depth_diff.x;
	physics.position.y += depth_diff.y;
}

void Collider::handle_platform_collision(Shape const& cell) {}

void Collider::handle_spike_collision(Shape const& cell) {
	if (hurtbox.overlaps(cell)) { spike_trigger = true; }
}

void Collider::handle_collider_collision(Shape const& collider) {
	if (flags.general.test(General::ignore_resolution)) { return; }
	if (!vicinity.overlaps(collider)) { return; }
	if (collision_depths) {
		if (collision_depths.value().crushed()) { return; }
	}

	flags.collision = {};

	// store all four mtvs
	mtvs.combined = predictive_combined.testCollisionGetMTV(predictive_combined, collider);
	mtvs.vertical = predictive_vertical.testCollisionGetMTV(predictive_vertical, collider);
	mtvs.horizontal = predictive_horizontal.testCollisionGetMTV(predictive_horizontal, collider);
	mtvs.actual = bounding_box.testCollisionGetMTV(bounding_box, collider);

	float vert_threshold = 5.5f;
	if (collision_depths) { collision_depths.value().calculate(*this, collider); }

	bool corner_collision{true};
	if (predictive_vertical.SAT(collider)) {
		mtvs.vertical.y < 0.f ? flags.collision.set(Collision::has_bottom_collision) : flags.collision.set(Collision::has_top_collision);
		//if (abs(mtvs.vertical.y > 0.001f)) { std::cout << "Vertical MTV reading: " << mtvs.vertical.y << "\n"; }
		if (flags.collision.test(Collision::has_bottom_collision) && physics.velocity.y > vert_threshold) {
			flags.state.set(State::just_landed);
			flags.animation.set(Animation::just_landed);
		} // for landing sound
		corner_collision = false;
		flags.external_state.set(ExternalState::collider_collision);
		flags.external_state.set(ExternalState::vert_collider_collision);

		if (flags.general.test(General::soft)) {
			correct_y(mtvs.vertical);
		} else {
			flags.collision.test(Collision::has_top_collision) ? correct_y(mtvs.vertical) : correct_y(mtvs.vertical);
		}
		if (flags.collision.test(Collision::has_top_collision)) { flags.external_state.set(ExternalState::jumped_into); }
	}
	if (predictive_horizontal.SAT(collider)) {
		mtvs.horizontal.x > 0.f ? flags.collision.set(Collision::has_left_collision) : flags.collision.set(Collision::has_right_collision);
		corner_collision = false;
		flags.dash.set(Dash::dash_cancel_collision);
		flags.external_state.set(ExternalState::collider_collision);
		flags.external_state.set(ExternalState::horiz_collider_collision);
		flags.external_state.reset(ExternalState::vert_collider_collision);
		correct_x(mtvs.horizontal);
	}
	if (predictive_combined.overlaps(collider) && corner_collision) {
		flags.collision.set(Collision::any_collision);
		flags.dash.set(Dash::dash_cancel_collision);
		flags.external_state.set(ExternalState::collider_collision);
		correct_corner(mtvs.combined);
	}
	if (wallslider.overlaps(collider)) { wallslider.vertices.at(0).x > collider.vertices.at(0).x ? flags.state.set(State::left_wallslide_collision) : flags.state.set(State::right_wallslide_collision); }

	if (jumpbox.SAT(collider)) {
		flags.state.set(State::grounded);
		flags.state.set(State::is_any_jump_collision);
	} else {
		flags.state.reset(State::grounded);
	}

	if (flags.external_state.test(ExternalState::vert_collider_collision)) { flags.external_state.reset(ExternalState::horiz_collider_collision); }
	flags.movement.reset(Movement::dashing);
	sync_components();
}

void Collider::update(automa::ServiceProvider& svc) {
	flags.external_state = {};
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
	box.setOutlineColor(sf::Color{255, 0, 0, 220});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw predictive horizontal
	box.setSize(predictive_horizontal.dimensions);
	box.setPosition(predictive_horizontal.position.x - cam.x, predictive_horizontal.position.y - cam.y);
	box.setOutlineColor(sf::Color{80, 0, 255, 220});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw predictive combined
	box.setSize(predictive_combined.dimensions);
	box.setPosition(predictive_combined.position.x - cam.x, predictive_combined.position.y - cam.y);
	box.setOutlineColor(sf::Color{255, 255, 80, 180});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw bounding box
	box.setSize(dimensions);
	box.setPosition(bounding_box.position.x - cam.x, bounding_box.position.y - cam.y);
	box.setFillColor(colors.local);
	box.setOutlineColor(sf::Color{255, 255, 255, 190});
	box.setOutlineThickness(-1);
	win.draw(box);

	// draw jump box
	box.setSize(jumpbox.dimensions);
	box.setPosition(jumpbox.position.x - cam.x, jumpbox.position.y - cam.y);
	box.setFillColor(sf::Color::Blue);
	box.setOutlineColor(sf::Color::Transparent);
	flags.state.test(State::is_any_jump_collision) ? box.setFillColor(sf::Color::Blue) : box.setFillColor(sf::Color::Yellow);
	win.draw(box);

	// draw hurtbox
	draw_hurtbox.setSize(sf::Vector2<float>{(float)hurtbox.dimensions.x, (float)hurtbox.dimensions.y});
	draw_hurtbox.setPosition(hurtbox.position.x - cam.x, hurtbox.position.y - cam.y);
	//win.draw(draw_hurtbox);

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
	win.draw(box);

	// draw physics position
	if (collision_depths) {
		box.setSize(vertical.dimensions);
		box.setPosition(vertical.position.x - cam.x, vertical.position.y - cam.y);
		collision_depths.value().vertical_squish() ? box.setFillColor(sf::Color::Green) : box.setFillColor(sf::Color::Red);
		box.setOutlineThickness(0);
		//win.draw(box); // draw physics position
		box.setSize(horizontal.dimensions);
		box.setPosition(horizontal.position.x - cam.x, horizontal.position.y - cam.y);
		collision_depths.value().horizontal_squish() ? box.setFillColor(sf::Color::Green) : box.setFillColor(sf::Color::Red);
		box.setOutlineThickness(0);
		//win.draw(box);
	}

	if (collision_depths) { collision_depths.value().render(bounding_box, win, cam); }

}
void Collider::reset() { flags.state = {}; }
void Collider::reset_ground_flags() {

	if (flags.state.test(State::is_any_jump_collision)) {
		flags.state.set(State::grounded);
	} else {
		flags.state.reset(State::grounded);
	}
}
bool Collider::on_ramp() const { return flags.state.test(State::on_ramp); }

bool Collider::has_horizontal_collision() const { return flags.collision.test(Collision::has_left_collision) || flags.collision.test(Collision::has_right_collision); }

bool Collider::has_left_collision() const { return flags.collision.test(Collision::has_left_collision); }

bool Collider::has_right_collision() const { return flags.collision.test(Collision::has_right_collision); }

bool Collider::has_vertical_collision() const { return flags.collision.test(Collision::has_top_collision) || flags.collision.test(Collision::has_bottom_collision); }

bool Collider::has_left_wallslide_collision() const { return flags.state.test(State::left_wallslide_collision); }

bool Collider::has_right_wallslide_collision() const { return flags.state.test(State::right_wallslide_collision); }

bool Collider::has_jump_collision() const { return grounded(); }

bool Collider::horizontal_squish() const { return collision_depths ? collision_depths.value().horizontal_squish() : false; }

bool Collider::vertical_squish() const { return collision_depths ? collision_depths.value().vertical_squish() : false; }

bool Collider::pushes(Collider& other) const {
	return (physics.position.x < other.physics.position.x && physics.velocity.x > 0.f) || (physics.position.x > other.physics.position.x && physics.velocity.x < 0.f);
}

sf::Vector2<float> Collider::get_average_tick_position() { return physics.previous_position; }

sf::Vector2<float> Collider::snap_to_grid(float size, float scale, float factor) {
	return sf::Vector2<float>{std::round((physics.position.x * size / factor) / (size * (scale / factor))), std::round((physics.position.y * size / factor) / (size * (scale / factor)))} *
		   scale;
}

float Collider::compute_length(sf::Vector2<float> const v) { return ccm::sqrt(v.x * v.x + v.y * v.y); }

} // namespace shape
