
#include "fornani/utils/Collider.hpp"
#include <ccmath/math/power/sqrt.hpp>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::shape {

constexpr auto vicinity_pad_v = 33.f;

Collider::Collider() {
	dimensions = sf::Vector2f{default_dim, default_dim};
	jumpbox.set_dimensions(sf::Vector2f(dimensions.x, default_jumpbox_height));
	hurtbox.set_dimensions(sf::Vector2f(dimensions.x - 8.f, dimensions.y - 8.f));
	sync_components();
}

Collider::Collider(sf::Vector2f dim, sf::Vector2f hbx_offset) : dimensions(dim), hurtbox_offset(hbx_offset) {
	bounding_box.set_dimensions(dim);
	jumpbox.set_dimensions(sf::Vector2f(dim.x, default_jumpbox_height));
	hurtbox.set_dimensions(sf::Vector2f(dim.x - 8.f, dim.y - 8.f + hurtbox_offset.y));
	sync_components();
}

void Collider::sync_components() {
	bounding_box.set_position(physics.position);
	bounding_box.set_dimensions(dimensions);
	vicinity.set_dimensions({dimensions.x + 2.f * vicinity_pad_v, dimensions.y + 2.f * vicinity_pad_v});
	wallslider.set_dimensions({dimensions.x + 2.f * wallslide_pad, dimensions.y * 0.6f});

	vertical.set_dimensions({1.f, dimensions.y - 2.f * depth_buffer});
	horizontal.set_dimensions({dimensions.x - 2.f * depth_buffer, 1.f});

	predictive_vertical.set_dimensions({dimensions.x - 2.f * vertical_detector_buffer, dimensions.y + 2.f * vertical_detector_buffer});
	predictive_horizontal.set_dimensions({dimensions.x + 2.f * horizontal_detector_buffer, dimensions.y - 3.f * horizontal_detector_buffer});
	predictive_combined.set_dimensions(dimensions);

	vicinity.set_position(sf::Vector2f{physics.position.x - vicinity_pad_v + physics.apparent_velocity().x, physics.position.y - vicinity_pad_v + physics.apparent_velocity().y});
	wallslider.set_position(sf::Vector2f{physics.position.x - wallslide_pad, physics.position.y + 2.f});
	predictive_vertical.set_position(sf::Vector2f{physics.position.x + vertical_detector_buffer, physics.position.y - vertical_detector_buffer + physics.apparent_velocity().y});
	predictive_horizontal.set_position(sf::Vector2f{physics.position.x - horizontal_detector_buffer + physics.apparent_velocity().x, physics.position.y + horizontal_detector_buffer});
	predictive_combined.set_position(sf::Vector2f{physics.position.x + physics.apparent_velocity().x, physics.position.y + physics.apparent_velocity().y});
	jumpbox.set_position(sf::Vector2f{physics.position.x, physics.position.y + dimensions.y});
	hurtbox.set_position(sf::Vector2f(physics.position.x + (dimensions.x * 0.5f) - (hurtbox.get_dimensions().x * 0.5f), physics.position.y + (dimensions.y * 0.5f) - (hurtbox.get_dimensions().y * 0.5f) - (hurtbox_offset.y * 0.5f)));
	vertical.set_position(sf::Vector2f{physics.position.x + dimensions.x * 0.5f - 0.5f, physics.position.y + depth_buffer});
	horizontal.set_position(sf::Vector2f{physics.position.x + depth_buffer, physics.position.y + dimensions.y * 0.5f - 0.5f});

	draw_hurtbox.setFillColor(sf::Color::Transparent);
	draw_hurtbox.setOutlineColor(sf::Color::Magenta);
	draw_hurtbox.setOutlineThickness(-1);
	draw_hurtbox.setSize(hurtbox.get_dimensions());
	draw_hurtbox.setPosition(hurtbox.get_position());
}

void Collider::handle_map_collision(world::Tile const& tile) {
	if (collision_depths) {
		if (collision_depths.value().crushed()) {
			NANI_LOG_DEBUG(m_logger, "Crushed!");
			collision_depths->print();
			return;
		}
	}

	flags.collision = {};
	auto const& cell = tile.bounding_box;

	// tile flags
	bool const is_ground_ramp = tile.is_ground_ramp();
	bool const is_ceiling_ramp = tile.is_ceiling_ramp();
	bool const is_plat = tile.is_platform() && (jumpbox.get_position().y > cell.get_position().y + 4 || physics.acceleration.y < 0.0f);
	bool const is_spike = tile.is_spike();
	bool const is_ramp = tile.is_ramp();

	// special tile types
	if (is_plat) { return; }
	if (is_spike) { return; }

	// store all four mtvs
	mtvs.combined = predictive_combined.get_MTV(predictive_combined, cell);
	mtvs.vertical = predictive_vertical.get_MTV(predictive_vertical, cell);
	mtvs.horizontal = predictive_horizontal.get_MTV(predictive_horizontal, cell);
	mtvs.actual = bounding_box.get_MTV(bounding_box, cell);

	// let's first settle all actual block collisions
	auto is_on_ramp = jumpbox.SAT(cell) && !flags.state.test(State::on_flat_surface) && !flags.movement.test(Movement::jumping) && physics.apparent_velocity().y > -0.001f && bottom() >= cell.top() - 1.f && tile.is_ground_ramp();
	if (!is_ramp) {
		if (collision_depths) { collision_depths.value().calculate(*this, cell); }
		bool vert{};
		if (predictive_vertical.SAT(cell)) {
			mtvs.vertical.y < 0.f ? flags.collision.set(Collision::has_bottom_collision) : flags.collision.set(Collision::has_top_collision);
			if (flags.collision.test(Collision::has_bottom_collision) && physics.apparent_velocity().y > vert_threshold) {
				flags.state.set(State::just_landed);
				flags.animation.set(Animation::just_landed);
			}
			flags.external_state.set(ExternalState::world_collision);
			flags.external_state.set(ExternalState::vert_world_collision);
			if (physics.apparent_velocity().y < 0.f && predictive_vertical.top() < cell.top()) {
			} else {
				correct_y(mtvs.vertical);
			}
			vert = true;
		}
		auto skip_the_corner = tile.ramp_adjacent();
		if (predictive_horizontal.SAT(cell) && !skip_the_corner && !vert) {
			mtvs.horizontal.x > 0.f ? flags.collision.set(Collision::has_left_collision) : flags.collision.set(Collision::has_right_collision);
			flags.dash.set(Dash::dash_cancel_collision);
			flags.external_state.set(ExternalState::world_collision);
			flags.external_state.set(ExternalState::horiz_world_collision);
			flags.external_state.reset(ExternalState::vert_world_collision);
			correct_x(mtvs.horizontal);
		}
		if (predictive_combined.SAT(cell) && !skip_the_corner) {
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
			flags.state.set(State::tickwise_ramp_collision);
			if (is_ground_ramp) {
				flags.external_state.set(ExternalState::on_ramp);
				physics.position.y -= ccm::abs(mtvs.actual.y);
				//  still zero this because of gravity
				if (!flags.movement.test(Movement::jumping) && bounding_box.bottom() <= cell.bottom()) {
					if (physics.apparent_velocity().y > vert_threshold) {
						flags.state.set(State::just_landed);
						flags.animation.set(Animation::just_landed);
					}
					physics.zero_y();
				}
			}
			if (is_ceiling_ramp) {
				tile.debug_flag = true;
				physics.position.y += ccm::abs(mtvs.combined.y) * 4.f;
				if (physics.apparent_velocity().y < 0.f) { physics.zero_y(); }
				flags.external_state.set(ExternalState::ceiling_ramp_hit);
				physics.acceleration.x *= 0.9f;
			}
			// cancel dash
			flags.dash.set(Dash::dash_cancel_collision);
			flags.external_state.set(ExternalState::world_collision);
		}
		// collider is on a ramp
		if (is_on_ramp) {
			flags.external_state.set(ExternalState::on_ramp);
			acceleration_multiplier = cell.get_radial_factor();
			auto to_the_right = physics.position.x + dimensions.x > cell.get_position().x + constants::f_cell_size;
			auto positive_input = to_the_right ? cell.get_height_at(physics.position.x + dimensions.x - cell.get_position().x) : cell.get_height_at(physics.position.x + dimensions.x - cell.get_position().x);
			if (tile.is_negative_ramp()) { maximum_ramp_height = std::max(maximum_ramp_height, cell.get_height_at(physics.position.x - cell.get_position().x)); }
			if (tile.is_positive_ramp()) { maximum_ramp_height = std::max(maximum_ramp_height, positive_input); }
			physics.position.y = cell.get_position().y + cell.get_dimensions().y - maximum_ramp_height - dimensions.y;
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
	// flags.external_state.reset(ExternalState::grounded);
	flags.external_state.reset(ExternalState::on_ramp);
	flags.external_state.reset(ExternalState::tile_debug_flag);
	flags.perma_state = {};
	flags.state.reset(State::tickwise_ramp_collision);

	auto& grid = map.get_middleground()->grid;
	auto top = map.get_index_at_position(vicinity.vertices.at(0));
	auto bottom = map.get_index_at_position(vicinity.vertices.at(3));
	auto right = map.get_index_at_position(vicinity.vertices.at(1)) - top;
	for (auto i{top}; i <= bottom; i += static_cast<std::size_t>(map.dimensions.x)) {
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

void Collider::correct_x(sf::Vector2f mtv) {
	if (flags.general.test(General::ignore_resolution)) { return; }
	auto xdist = predictive_horizontal.get_position().x + horizontal_detector_buffer - physics.position.x;
	auto correction = xdist + mtv.x;
	physics.position.x += correction;
	physics.zero_x();
}

void Collider::correct_y(sf::Vector2f mtv) {
	if (flags.general.test(General::ignore_resolution)) { return; }
	// for large mtv values, overcorrect to prevent clipping
	if (ccm::abs(mtv.x) > 12.f || ccm::abs(mtv.y) > 12.f) {
		mtv.x = ccm::abs(mtv.y) > 0 ? mtv.y : mtv.x;
		mtv.y = ccm::abs(mtv.x) > 0 ? mtv.x : mtv.y;
	}
	auto ydist = predictive_vertical.get_position().y + vertical_detector_buffer - physics.position.y;
	auto correction = ydist + mtv.y;
	physics.position.y += correction;
	physics.zero_y();
}

void Collider::correct_x_y(sf::Vector2f mtv) {
	if (flags.general.test(General::ignore_resolution)) { return; }
	auto xdist = predictive_combined.get_position().x - physics.position.x;
	auto correction = xdist + mtv.x;
	physics.position.x += correction;
	physics.zero_x();
	auto ydist = predictive_combined.get_position().y - physics.position.y;
	correction = ydist + mtv.y;
	physics.position.y += correction;
	physics.zero_y();
}

void Collider::correct_corner(sf::Vector2f mtv) {
	if (flags.general.test(General::ignore_resolution)) { return; }
	if (ccm::abs(mtv.x) >= ccm::abs(mtv.y)) {
		physics.position.x = predictive_combined.get_position().x + mtv.x;
		physics.zero_x();
	} else {
		auto ydist = predictive_combined.get_position().y - physics.position.y;
		physics.position.y = predictive_combined.get_position().y + mtv.y;
		physics.zero_y();
	}
}

void Collider::resolve_depths() {
	if (!collision_depths) { return; }
	auto downward = collision_depths.value().top_depth() > collision_depths.value().bottom_depth();
	auto rightward = collision_depths.value().left_depth() > collision_depths.value().right_depth();
	sf::Vector2f depth_diff{};
	depth_diff.y = (collision_depths.value().top_depth() + collision_depths.value().bottom_depth()) * 0.5f;
	depth_diff.x = (collision_depths.value().left_depth() + collision_depths.value().right_depth()) * 0.5f;
	if (depth_diff.x < 8.f && depth_diff.y < 8.f) { return; }
	depth_diff.y *= downward ? 1.f : -1.f;
	depth_diff.x *= rightward ? 1.f : -1.f;
	physics.position.x += depth_diff.x;
	physics.position.y += depth_diff.y;
}

bool Collider::handle_collider_collision(Shape const& collider, bool soft, sf::Vector2f velocity) {
	auto ret{false};
	if (soft) {
		if (!vicinity.overlaps(collider)) { return ret; }
		mtvs.actual = bounding_box.get_MTV(bounding_box, collider);
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
	mtvs.combined = predictive_combined.get_MTV(predictive_combined, collider);
	mtvs.vertical = predictive_vertical.get_MTV(predictive_vertical, collider);
	mtvs.horizontal = predictive_horizontal.get_MTV(predictive_horizontal, collider);
	mtvs.actual = bounding_box.get_MTV(bounding_box, collider);
	if (!util::same_sign(velocity.y, mtvs.vertical.y)) { velocity.y = 0.f; }
	if (!util::same_sign(velocity.x, mtvs.horizontal.x)) { velocity.x = 0.f; }

	if (collision_depths) { collision_depths.value().calculate(*this, collider); }

	bool corner_collision{true};
	if (predictive_vertical.SAT(collider)) {
		mtvs.vertical.y < 0.f ? flags.collision.set(Collision::has_bottom_collision) : flags.collision.set(Collision::has_top_collision);
		if (flags.collision.test(Collision::has_bottom_collision) && physics.apparent_velocity().y > vert_threshold) {
			flags.state.set(State::just_landed);
			flags.animation.set(Animation::just_landed);
		}
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
		if (jumpbox.get_position().y > collider.physics.position.y + 4.f || physics.acceleration.y < 0.0f) { return; }
	}
	if (handle_collider_collision(collider.bounding_box, soft, collider.physics.apparent_velocity() * 4.f)) {
		if (momentum) { physics.forced_momentum = collider.physics.position - collider.physics.previous_position; }
	}
	if (jumpbox.overlaps(collider.bounding_box)) { flags.external_state.set(ExternalState::grounded); }
}

void Collider::update(automa::ServiceProvider& svc, bool simple) {
	if (!on_ramp()) { acceleration_multiplier = 1.f; }
	flags.external_state = {};
	adjust_acceleration();
	if (!flags.general.test(General::no_move)) { simple ? physics.simple_update() : physics.update(svc); }
	sync_components();
	flags.state.reset(State::just_collided);
	physics.gravity = flags.state.test(State::grounded) ? 0.0f : stats.GRAV;
	flags.state.test(State::grounded) ? physics.flags.set(components::State::grounded) : physics.flags.reset(components::State::grounded);
}

void Collider::render(sf::RenderWindow& win, sf::Vector2f cam) {

	// draw predictive vertical
	box.setSize(predictive_vertical.get_dimensions());
	box.setPosition(predictive_vertical.get_position() - cam);
	box.setOutlineColor(sf::Color{255, 0, 0, 220});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	// win.draw(box);

	// draw predictive horizontal
	box.setSize(predictive_horizontal.get_dimensions());
	box.setPosition(predictive_horizontal.get_position() - cam);
	box.setOutlineColor(sf::Color{80, 0, 255, 220});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	// win.draw(box);

	// draw predictive combined
	box.setSize(predictive_combined.get_dimensions());
	box.setPosition(predictive_combined.get_position() - cam);
	box.setOutlineColor(sf::Color{255, 255, 80, 180});
	box.setOutlineThickness(-1);
	box.setFillColor(sf::Color::Transparent);
	// win.draw(box);

	// draw bounding box
	box.setSize(dimensions);
	box.setPosition(bounding_box.get_position() - cam);
	box.setFillColor(colors.local);
	flags.state.test(State::on_flat_surface) ? box.setOutlineColor(sf::Color{0, 255, 0, 190}) : box.setOutlineColor(sf::Color{255, 0, 0, 190});
	box.setOutlineThickness(-1);
	win.draw(box);

	// draw jump box
	box.setSize(jumpbox.get_dimensions());
	box.setPosition(jumpbox.get_position() - cam);
	box.setFillColor(sf::Color::Blue);
	box.setOutlineColor(sf::Color::Transparent);
	flags.external_state.test(ExternalState::grounded) ? box.setFillColor(sf::Color::Blue) : box.setFillColor(sf::Color::Yellow);
	// win.draw(box);

	// draw hurtbox
	draw_hurtbox.setSize(sf::Vector2f{hurtbox.get_dimensions()});
	draw_hurtbox.setPosition(hurtbox.get_position() - cam);
	draw_hurtbox.setOutlineColor(colors::ui_white);
	draw_hurtbox.setOutlineThickness(-1.f);
	// win.draw(draw_hurtbox);

	// draw vicinity
	box.setSize(sf::Vector2f{vicinity.get_dimensions()});
	box.setPosition(vicinity.get_position() - cam);
	box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(sf::Color{120, 60, 80, 180});
	box.setOutlineThickness(-1);
	win.draw(box);

	// draw wallslider
	box.setSize(sf::Vector2f{wallslider.get_dimensions()});
	box.setPosition(wallslider.get_position() - cam);
	has_left_wallslide_collision() || has_right_wallslide_collision() ? box.setFillColor(sf::Color::Blue) : box.setFillColor(sf::Color::Transparent);
	box.setOutlineColor(sf::Color{60, 60, 180, 100});
	box.setOutlineThickness(-1);
	// win.draw(box);

	// draw physics position
	if (collision_depths) {
		box.setSize(vertical.get_dimensions());
		box.setPosition(vertical.get_position() - cam);
		collision_depths.value().vertical_squish() ? box.setFillColor(sf::Color::Green) : box.setFillColor(sf::Color::Red);
		box.setOutlineThickness(0);
		// win.draw(box); // draw physics position
		box.setSize(horizontal.get_dimensions());
		box.setPosition(horizontal.get_position() - cam);
		collision_depths.value().horizontal_squish() ? box.setFillColor(sf::Color::Green) : box.setFillColor(sf::Color::Red);
		box.setOutlineThickness(0);
		// win.draw(box);
	}

	if (collision_depths) { collision_depths.value().render(bounding_box, win, cam); }
}
void Collider::set_position(sf::Vector2f pos) {
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

void Collider::adjust_acceleration() {
	physics.multiply_acceleration(acceleration_multiplier, {1.f, 0.f});
	acceleration_multiplier = 1.f;
}

void Collider::fix() { flags.general.set(General::no_move); }

bool Collider::on_ramp() const { return flags.external_state.test(ExternalState::on_ramp); }

bool Collider::has_horizontal_collision() const { return flags.collision.test(Collision::has_left_collision) || flags.collision.test(Collision::has_right_collision); }

bool Collider::has_left_collision() const { return flags.collision.test(Collision::has_left_collision); }

bool Collider::has_right_collision() const { return flags.collision.test(Collision::has_right_collision); }

bool Collider::has_vertical_collision() const { return flags.collision.test(Collision::has_top_collision) || flags.collision.test(Collision::has_bottom_collision); }

bool Collider::has_left_wallslide_collision() const { return flags.state.test(State::left_wallslide_collision); }

bool Collider::has_right_wallslide_collision() const { return flags.state.test(State::right_wallslide_collision); }

bool Collider::has_jump_collision() const { return grounded(); }

bool Collider::horizontal_squish() const { return collision_depths ? collision_depths.value().horizontal_squish() : false; }

bool Collider::vertical_squish() const { return collision_depths ? collision_depths.value().vertical_squish() : false; }

bool Collider::pushes(Collider& other) const { return (physics.position.x < other.physics.position.x && physics.apparent_velocity().x > 0.f) || (physics.position.x > other.physics.position.x && physics.apparent_velocity().x < 0.f); }

sf::Vector2f Collider::snap_to_grid(float size, float scale, float factor) {
	return sf::Vector2f{std::round((physics.position.x * size / factor) / (size * (scale / factor))), std::round((physics.position.y * size / factor) / (size * (scale / factor)))} * scale;
}

} // namespace fornani::shape
