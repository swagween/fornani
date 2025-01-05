
#include "Collider.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include <ccmath/math/power/sqrt.hpp>
#include "../utils/Math.hpp"

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
	wallslider.set_normals();
	jumpbox.set_normals();

	vertical.dimensions.x = 1.f;
	vertical.dimensions.y = dimensions.y - 2.f * depth_buffer;
	horizontal.dimensions.x = dimensions.x - 2.f * depth_buffer;
	horizontal.dimensions.y = 1.f;
	horizontal.set_normals();
	vertical.set_normals();

	predictive_vertical.dimensions.x = dimensions.x - 2.f * vertical_detector_buffer;
	predictive_vertical.dimensions.y = dimensions.y + 2.f * vertical_detector_buffer;
	predictive_vertical.set_normals();
	predictive_horizontal.dimensions.x = dimensions.x + 2.f * horizontal_detector_buffer;
	predictive_horizontal.dimensions.y = dimensions.y - 3.f * horizontal_detector_buffer;
	predictive_horizontal.set_normals();
	predictive_combined.dimensions = dimensions;
	predictive_combined.set_normals();

	vicinity.set_position(sf::Vector2<float>{physics.position.x - vicinity_pad + physics.apparent_velocity().x, physics.position.y - vicinity_pad + physics.apparent_velocity().y});
	wallslider.set_position(sf::Vector2<float>{physics.position.x - wallslide_pad, physics.position.y + 2.f});
	predictive_vertical.set_position(sf::Vector2<float>{physics.position.x + vertical_detector_buffer, physics.position.y - vertical_detector_buffer + physics.apparent_velocity().y});
	predictive_horizontal.set_position(sf::Vector2<float>{physics.position.x - horizontal_detector_buffer + physics.apparent_velocity().x, physics.position.y + horizontal_detector_buffer});
	predictive_combined.set_position(sf::Vector2<float>{physics.position.x + physics.apparent_velocity().x, physics.position.y + physics.apparent_velocity().y});
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
	bool const is_ground_ramp = tile.is_ground_ramp();
	bool const is_ceiling_ramp = tile.is_ceiling_ramp();
	bool const is_plat = tile.is_platform() && (jumpbox.position.y > cell.position.y + 4 || physics.acceleration.y < 0.0f);
	bool const is_spike = tile.is_spike();
	bool const is_ramp = tile.is_ramp();

	// special tile types
	if (is_plat) {
		handle_platform_collision(cell);
		return;
	}
	if (is_spike) { return; }

	// store all four mtvs
	mtvs.combined = predictive_combined.testCollisionGetMTV(predictive_combined, cell);
	mtvs.vertical = predictive_vertical.testCollisionGetMTV(predictive_vertical, cell);
	mtvs.horizontal = predictive_horizontal.testCollisionGetMTV(predictive_horizontal, cell);
	mtvs.actual = bounding_box.testCollisionGetMTV(bounding_box, cell);

	// let's first settle all actual block collisions
	if (!is_ramp) {
		if (collision_depths) { collision_depths.value().calculate(*this, cell); }
		bool corner_collision{};
		bool vert{};
		if (predictive_vertical.SAT(cell)) {
			mtvs.vertical.y < 0.f ? flags.collision.set(Collision::has_bottom_collision) : flags.collision.set(Collision::has_top_collision);
			if (flags.collision.test(Collision::has_bottom_collision) && physics.apparent_velocity().y > vert_threshold) {
				flags.state.set(State::just_landed);
				flags.animation.set(Animation::just_landed);
			} // for landing sound
			flags.external_state.set(ExternalState::world_collision);
			flags.external_state.set(ExternalState::vert_world_collision);
			if (physics.apparent_velocity().y < 0.f && predictive_vertical.top() < cell.top()) {
			} else {
				correct_y(mtvs.vertical);
			}
			vert = true;
		}
		if (predictive_horizontal.SAT(cell) && !tile.ramp_adjacent() && !vert) {
			mtvs.horizontal.x > 0.f ? flags.collision.set(Collision::has_left_collision) : flags.collision.set(Collision::has_right_collision);
			flags.dash.set(Dash::dash_cancel_collision);
			flags.external_state.set(ExternalState::world_collision);
			flags.external_state.set(ExternalState::horiz_world_collision);
			flags.external_state.reset(ExternalState::vert_world_collision);
			correct_x(mtvs.horizontal);
		}
		if (predictive_combined.SAT(cell) && !tile.ramp_adjacent()) {
			// if (abs(mtvs.combined.x > 0.0001f)) { std::cout << "Combined MTV reading: " << mtvs.combined.x << "\n"; }
			flags.collision.set(Collision::any_collision);
			flags.dash.set(Dash::dash_cancel_collision);
			flags.external_state.set(ExternalState::world_collision);
			correct_corner(mtvs.combined);
		}
		if (flags.external_state.test(ExternalState::vert_world_collision)) { flags.external_state.reset(ExternalState::horiz_world_collision); }
	}
	// don't fix ramp position unless we're only colliding with ramps
	// now let's settle ramp collisions. remember, the collider has already been resolved from any previous cell collision
	tile.debug_flag = false;
	if (is_ramp) {
		flags.external_state.set(ExternalState::tile_debug_flag);
		// ground ramp
		// only handle ramp collisions if the bounding_box is colliding with it
		if (bounding_box.SAT(cell)) {
			if (physics.apparent_velocity().y > vert_threshold) {
				flags.state.set(State::just_landed);
				flags.animation.set(Animation::just_landed);
			}
			flags.state.set(State::tickwise_ramp_collision);
			if (is_ground_ramp) {
				flags.external_state.set(ExternalState::on_ramp);
				physics.position.y -= abs(mtvs.actual.y);
				//flags.state.set(State::on_flat_surface);
				// still zero this because of gravity
				if (!flags.movement.test(Movement::jumping)) {
					physics.zero_y();
				}
				// std::cout << "\nGround ramp collision with MTV y of: " << mtvs.actual.y;
				//if (mtvs.actual.y > 4.f) { physics.position.y -= mtvs.actual.y; } // player gets stuck in a thin ramp
			}
			if (is_ceiling_ramp) {
				tile.debug_flag = true;
				physics.position.y += abs(mtvs.combined.y) * 4.f;
				if (physics.apparent_velocity().y < 0.f) { physics.zero_y(); }
				flags.external_state.set(ExternalState::ceiling_ramp_hit);
			}
			// cancel dash
			flags.dash.set(Dash::dash_cancel_collision);
			flags.external_state.set(ExternalState::world_collision);
		}

		if (jumpbox.SAT(cell) && !flags.state.test(State::on_flat_surface) && !flags.movement.test(Movement::jumping) && physics.apparent_velocity().y > -0.001f && bottom() >= cell.top() - 1.f && tile.is_ground_ramp()) {
			if (tile.is_negative_ramp()) { maximum_ramp_height = std::max(maximum_ramp_height, cell.get_height_at(abs(physics.position.x - cell.position.x))); }
			if (tile.is_positive_ramp()) { maximum_ramp_height = std::max(maximum_ramp_height, cell.get_height_at(abs(physics.position.x + dimensions.x - cell.position.x))); }
			physics.position.y = cell.position.y + cell.dimensions.y - maximum_ramp_height - dimensions.y;
			if ((physics.apparent_velocity().x >= 0.f && tile.is_negative_ramp()) || (physics.apparent_velocity().x <= 0.f && tile.is_positive_ramp())) { flags.perma_state.set(PermaFlags::downhill); }
		} else if (flags.state.test(State::on_flat_surface)) {
			flags.perma_state.reset(PermaFlags::downhill);
		}
	}
	if (!is_ramp && jumpbox.SAT(cell) && !tile.covered()) { flags.state.set(State::on_flat_surface); }
	if (!is_ramp && wallslider.overlaps(cell)) { wallslider.vertices.at(0).x > cell.vertices.at(0).x ? flags.state.set(State::left_wallslide_collision) : flags.state.set(State::right_wallslide_collision); }

	// long-winded, but I want to reserve SAT for colliders that actually need it
	if (flags.general.test(General::complex)) {
		if (jumpbox.SAT(cell)) {
			flags.external_state.reset(ExternalState::ceiling_ramp_hit);
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
			flags.external_state.reset(ExternalState::ceiling_ramp_hit);
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
	flags.external_state.reset(ExternalState::on_ramp);
	flags.external_state.reset(ExternalState::tile_debug_flag);
	flags.perma_state = {};
	flags.state.reset(State::tickwise_ramp_collision);

	auto& grid = map.get_layers().at(world::MIDDLEGROUND).grid;
	auto top = map.get_index_at_position(vicinity.vertices.at(0));
	auto bottom = map.get_index_at_position(vicinity.vertices.at(3));
	auto right = map.get_index_at_position(vicinity.vertices.at(1)) - top;
	for (auto i{top}; i <= bottom; i += static_cast<size_t>(map.dimensions.x)) {
		auto left{0};
		for (auto j{left}; j <= right; ++j) {
			auto index = i + j;
			if (index >= grid.cells.size() || index < 0) { continue; }
			auto& cell = grid.get_cell(static_cast<int>(index));
			if (!cell.is_collidable()) { continue; }
			cell.collision_check = true;
			handle_map_collision(cell);
		}
	}
	maximum_ramp_height = 0.f;
	flags.state.reset(State::on_flat_surface);
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

bool Collider::handle_collider_collision(Shape const& collider, bool soft, sf::Vector2<float> velocity) {
	auto ret{false};
	if (soft) {
		if (!vicinity.overlaps(collider)) { return ret; }
		mtvs.actual = bounding_box.testCollisionGetMTV(bounding_box, collider);
		if (bounding_box.SAT(collider)) { physics.position += mtvs.actual * 0.01f; }
		sync_components();
		return ret;
	}
	if (flags.general.test(General::ignore_resolution)) { return ret; }
	if (!vicinity.overlaps(collider)) { return ret; }
	if (collision_depths) {
		if (collision_depths.value().crushed()) { return ret; }
	}

	flags.collision = {};

	// store all four mtvs
	mtvs.combined = predictive_combined.testCollisionGetMTV(predictive_combined, collider);
	mtvs.vertical = predictive_vertical.testCollisionGetMTV(predictive_vertical, collider);
	mtvs.horizontal = predictive_horizontal.testCollisionGetMTV(predictive_horizontal, collider);
	mtvs.actual = bounding_box.testCollisionGetMTV(bounding_box, collider);
	if (!util::same_sign(velocity.y, mtvs.vertical.y)) { velocity.y = 0.f; }
	if (!util::same_sign(velocity.x, mtvs.horizontal.x)) { velocity.x = 0.f; }

	if (collision_depths) { collision_depths.value().calculate(*this, collider); }

	bool corner_collision{true};
	if (predictive_vertical.SAT(collider)) {
		mtvs.vertical.y < 0.f ? flags.collision.set(Collision::has_bottom_collision) : flags.collision.set(Collision::has_top_collision);
		// if (abs(mtvs.vertical.y > 0.001f)) { std::cout << "Vertical MTV reading: " << mtvs.vertical.y << "\n"; }
		if (flags.collision.test(Collision::has_bottom_collision) && physics.apparent_velocity().y > vert_threshold) {
			flags.state.set(State::just_landed);
			flags.animation.set(Animation::just_landed);
		} // for landing sound
		corner_collision = false;
		flags.external_state.set(ExternalState::collider_collision);
		flags.external_state.set(ExternalState::vert_collider_collision);

		if (flags.general.test(General::soft)) {
			correct_y(mtvs.vertical + velocity);
		} else {
			if (flags.collision.test(Collision::has_top_collision)) {
				flags.external_state.set(ExternalState::jumped_into);
				correct_y(mtvs.vertical + velocity);
			} else {
				correct_y(mtvs.vertical + velocity);
			}
		}
	}
	if (predictive_horizontal.SAT(collider)) {
		mtvs.horizontal.x > 0.f ? flags.collision.set(Collision::has_left_collision) : flags.collision.set(Collision::has_right_collision);
		corner_collision = false;
		flags.dash.set(Dash::dash_cancel_collision);
		flags.external_state.set(ExternalState::collider_collision);
		flags.external_state.set(ExternalState::horiz_collider_collision);
		flags.external_state.reset(ExternalState::vert_collider_collision);
		correct_x(mtvs.horizontal + velocity);
	}
	if (predictive_combined.overlaps(collider) && corner_collision) {
		flags.collision.set(Collision::any_collision);
		flags.dash.set(Dash::dash_cancel_collision);
		flags.external_state.set(ExternalState::collider_collision);
		correct_corner(mtvs.combined + velocity);
	}
	if (wallslider.overlaps(collider)) { wallslider.vertices.at(0).x > collider.vertices.at(0).x ? flags.state.set(State::left_wallslide_collision) : flags.state.set(State::right_wallslide_collision); }

	if (jumpbox.SAT(collider)) {
		flags.state.set(State::grounded);
		flags.state.set(State::is_any_jump_collision);
		ret = true;
	} else {
		flags.state.reset(State::grounded);
	}

	if (flags.external_state.test(ExternalState::vert_collider_collision)) { flags.external_state.reset(ExternalState::horiz_collider_collision); }
	flags.movement.reset(Movement::dashing);
	sync_components();
	return ret;
}

void Collider::handle_collider_collision(Collider const& collider, bool soft, bool momentum) {
	if (!vicinity.overlaps(collider.bounding_box)) { return; }
	if (collider.flags.general.test(General::top_only_collision)) {
		if (jumpbox.position.y > collider.physics.position.y + 4.f || physics.acceleration.y < 0.0f) { return; }
	}
	if (handle_collider_collision(collider.bounding_box, soft, collider.physics.apparent_velocity() * 4.f)) {
		if (momentum) { physics.forced_momentum = collider.physics.position - collider.physics.previous_position; }
	}
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
	box.setPosition(predictive_vertical.position - cam);
	box.setOutlineColor(sf::Color{255, 0, 0, 220});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw predictive horizontal
	box.setSize(predictive_horizontal.dimensions);
	box.setPosition(predictive_horizontal.position - cam);
	box.setOutlineColor(sf::Color{80, 0, 255, 220});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw predictive combined
	box.setSize(predictive_combined.dimensions);
	box.setPosition(predictive_combined.position - cam);
	box.setOutlineColor(sf::Color{255, 255, 80, 180});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	win.draw(box);

	// draw bounding box
	box.setSize(dimensions);
	box.setPosition(bounding_box.position - cam);
	box.setFillColor(colors.local);
	flags.state.test(State::on_flat_surface) ? box.setOutlineColor(sf::Color{0, 255, 0, 190}) : box.setOutlineColor(sf::Color{255, 0, 0, 190});
	box.setOutlineThickness(-1);
	win.draw(box);

	// draw jump box
	box.setSize(jumpbox.dimensions);
	box.setPosition(jumpbox.position - cam);
	box.setFillColor(sf::Color::Blue);
	box.setOutlineColor(sf::Color::Transparent);
	flags.external_state.test(ExternalState::grounded) ? box.setFillColor(sf::Color::Blue) : box.setFillColor(sf::Color::Yellow);
	win.draw(box);

	// draw hurtbox
	draw_hurtbox.setSize(sf::Vector2<float>{(float)hurtbox.dimensions.x, (float)hurtbox.dimensions.y});
	draw_hurtbox.setPosition(hurtbox.position - cam);
	//win.draw(draw_hurtbox);

	// draw vicinity
	box.setSize(sf::Vector2<float>{(float)vicinity.dimensions.x, (float)vicinity.dimensions.y});
	box.setPosition(vicinity.position - cam);
	box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(sf::Color{120, 60, 80, 180});
	box.setOutlineThickness(-1);
	win.draw(box);

	// draw wallslider
	box.setSize(sf::Vector2<float>{(float)wallslider.dimensions.x, (float)wallslider.dimensions.y});
	box.setPosition(wallslider.position - cam);
	has_left_wallslide_collision() || has_right_wallslide_collision() ? box.setFillColor(sf::Color::Blue) : box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(sf::Color{60, 60, 180, 100});
	box.setOutlineThickness(-1);
	win.draw(box);

	// draw physics position
	if (collision_depths) {
		box.setSize(vertical.dimensions);
		box.setPosition(vertical.position - cam);
		collision_depths.value().vertical_squish() ? box.setFillColor(sf::Color::Green) : box.setFillColor(sf::Color::Red);
		box.setOutlineThickness(0);
		//win.draw(box); // draw physics position
		box.setSize(horizontal.dimensions);
		box.setPosition(horizontal.position - cam);
		collision_depths.value().horizontal_squish() ? box.setFillColor(sf::Color::Green) : box.setFillColor(sf::Color::Red);
		box.setOutlineThickness(0);
		//win.draw(box);
	}

	if (collision_depths) { collision_depths.value().render(bounding_box, win, cam); }

}
void Collider::set_position(sf::Vector2<float> pos) {
	physics.position = pos;
	sync_components();
}
void Collider::reset() { flags.state = {}; }
void Collider::reset_ground_flags() {

	if (flags.state.test(State::is_any_jump_collision)) {
		flags.state.set(State::grounded);
	} else {
		flags.state.reset(State::grounded);
	}
}

void Collider::set_top_only() { flags.general.set(General::top_only_collision); }

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
	return (physics.position.x < other.physics.position.x && physics.apparent_velocity().x > 0.f) || (physics.position.x > other.physics.position.x && physics.apparent_velocity().x < 0.f);
}

sf::Vector2<float> Collider::get_average_tick_position() { return physics.previous_position; }

sf::Vector2<float> Collider::snap_to_grid(float size, float scale, float factor) {
	return sf::Vector2<float>{std::round((physics.position.x * size / factor) / (size * (scale / factor))), std::round((physics.position.y * size / factor) / (size * (scale / factor)))} *
		   scale;
}

float Collider::compute_length(sf::Vector2<float> const v) { return ccm::sqrt(v.x * v.x + v.y * v.y); }

} // namespace shape
