#include "fornani/world/Platform.hpp"
#include <fornani/utils/Math.hpp>
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Platform::Platform(automa::ServiceProvider& svc, sf::Vector2f position, sf::Vector2f dimensions, float extent, std::string_view specifications, float start_point, int style)
	: shape::Collider(dimensions, position), path_position(start_point), sprite{svc.assets.get_texture("platforms")}, switch_up{3} {

	auto const& in_data = svc.data.platform[specifications];
	if (in_data["sticky"].as_bool()) { flags.attributes.set(PlatformAttributes::sticky); }
	if (in_data["loop"].as_bool()) { flags.attributes.set(PlatformAttributes::loop); }
	if (in_data["repeating"].as_bool()) { flags.attributes.set(PlatformAttributes::repeating); }
	if (in_data["player_activated"].as_bool()) { flags.attributes.set(PlatformAttributes::player_activated); }
	if (in_data["player_controlled"].as_bool()) { flags.attributes.set(PlatformAttributes::player_controlled); }
	if (in_data["ease"].as_bool()) { flags.attributes.set(PlatformAttributes::ease); }

	metrics.speed = in_data["speed"].as<float>();

	for (auto& point : in_data["track"].as_array()) {
		track.push_back({position.x + (point[0].as<float>() * extent), position.y + (point[1].as<float>() * extent)});
		if (point[0].as<float>() > 0.f) { flags.attributes.set(PlatformAttributes::side_to_side); }
		if (point[1].as<float>() > 0.f) { flags.attributes.set(PlatformAttributes::up_down); }
	}
	track_shape.setPointCount(track.size());
	if (flags.attributes.test(PlatformAttributes::loop)) { track.push_back(track.at(0)); }

	for (std::size_t x = 0; x < track.size() - 1; ++x) {
		path_length += (track[x + 1] - track[x]).length();
		track_shape.setPoint(x, track[x] + dimensions * 0.5f);
	}

	track_shape.setFillColor(sf::Color::Transparent);
	track_shape.setOutlineColor(sf::Color(135, 132, 149, 140));
	track_shape.setOutlineThickness(2);

	direction.lnr = LNR::neutral;
	direction.und = UND::neutral;

	// for collision handling
	if (track.size() > 1) {
		native_direction.lnr = flags.attributes.test(PlatformAttributes::side_to_side) ? LNR::left : LNR::neutral;
		native_direction.und = flags.attributes.test(PlatformAttributes::up_down) ? UND::down : UND::neutral;
	}

	counter.start();

	// set visuals
	animation.set_params({0, 4, 16, -1});
	auto scaled_dim = dimensions / constants::f_cell_size;
	if (scaled_dim.x == 1) { offset = {0, 0}; }
	if (scaled_dim.x == 2) { offset = {16, 0}; }
	if (scaled_dim.x == 3) { offset = {0, 16}; }
	if (scaled_dim.y == 2) { offset = {0, 32}; }
	if (scaled_dim.y == 3) { offset = {0, 64}; }
	switch_up.start();
	sprite.setScale(constants::f_scale_vec);

	auto edge_start = 0.f;
	for (std::size_t x = 0; x < track.size() - 1; ++x) {
		auto const start = track[x];
		auto const end = track[x + 1];
		auto const len = (end - start).length();
		if (auto const edge_end = edge_start + (len / path_length); edge_end >= path_position) {
			constexpr auto skip_value{32.f};
			if (flags.attributes.test(PlatformAttributes::ease)) {
				physics.position.x = util::ease_in_out(start.x, end.x, (path_position - edge_start) / (edge_end - edge_start));
				physics.position.y = util::ease_in_out(start.y, end.y, (path_position - edge_start) / (edge_end - edge_start));
			} else {
				physics.position.x = std::lerp(start.x, end.x, (path_position - edge_start) / (edge_end - edge_start));
				physics.position.y = std::lerp(start.y, end.y, (path_position - edge_start) / (edge_end - edge_start));
			}
		}
	}
	sync_components();
}

void Platform::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	auto const old_position = physics.position;
	Collider::update(svc);
	auto edge_start = 0.f;
	if (player.get_collider().jumped_into() && physics.velocity.y > 0.f) { player.get_collider().physics.apply_force(physics.velocity * 8.f); }
	player.on_crush(map);
	for (auto const& enemy : map.enemy_catalog.enemies) {
		if (enemy->has_map_collision()) { enemy->get_collider().handle_collider_collision(*this); }
		enemy->on_crush(map);
	}
	switch_up.update();

	// map changes

	// platform changes
	handle_collider_collision(player.get_collider());
	for (auto& breakable : map.breakables) { handle_collider_collision(breakable->get_hurtbox()); }
	for (auto& pushable : map.pushables) {
		// platform should reverse direction upon hitting the sides or top of a pushable
		if (!pushable->get_collider().jumpbox.overlaps(bounding_box)) { handle_collider_collision(pushable->get_hurtbox()); }
	}
	for (auto& block : map.switch_blocks) {
		if (block->on()) { handle_collider_collision(block->get_hurtbox()); }
	}
	// handle platform collisions
	if (!switch_up.running()) {
		for (auto& platform : map.platforms) {
			if (&platform != this && native_direction.lnr != platform.native_direction.lnr) { handle_collider_collision(platform.hurtbox, true); }
		}
	}
	// init direction to oppose player
	direction.lnr = player.controller.direction.lnr == LNR::left ? LNR::right : LNR::left;

	if (flags.state.test(PlatformState::moving) && !switch_up.running()) {
		if (native_direction.lnr == LNR::left) {
			if (Collider::flags.external_state.consume(shape::ExternalState::horiz_collider_collision)) { switch_directions(); }
		} else {
			if (Collider::flags.external_state.consume(shape::ExternalState::vert_collider_collision)) { switch_directions(); }
		}
	}

	for (std::size_t x = 0; x < track.size() - 1; ++x) {
		auto const start = track[x];
		auto const end = track[x + 1];
		auto const len = (end - start).length();
		if (auto const edge_end = edge_start + (len / path_length); edge_end >= path_position) {
			constexpr auto skip_value{16.f};
			if (flags.attributes.test(PlatformAttributes::ease)) {
				physics.position.x = util::ease_in_out(start.x, end.x, (path_position - edge_start) / (edge_end - edge_start));
				physics.position.y = util::ease_in_out(start.y, end.y, (path_position - edge_start) / (edge_end - edge_start));
			} else {
				physics.position.x = std::lerp(start.x, end.x, (path_position - edge_start) / (edge_end - edge_start));
				physics.position.y = std::lerp(start.y, end.y, (path_position - edge_start) / (edge_end - edge_start));
			}
			physics.velocity = physics.position - old_position;
			physics.real_velocity = physics.velocity;
			// set direction
			direction.lnr = physics.velocity.x > 0.0f ? LNR::right : LNR::left;
			direction.und = physics.velocity.y > 0.0f ? UND::down : UND::up;

			// this stuff doesn't really belong here and it's very confusing to read, but it works
			if (player.get_collider().jumpbox.overlaps(bounding_box) && !player.get_collider().perma_grounded() && is_sticky() &&
				!(player.get_collider().has_left_wallslide_collision() || player.get_collider().has_right_wallslide_collision())) {
				if (!(abs(physics.velocity.x) > skip_value || abs(physics.velocity.y) > skip_value)) { player.get_collider().physics.forced_momentum = physics.position - old_position; }
			}
			//

			break;
		} else {
			edge_start = edge_end;
		}
	}
	if (player.controller.direction.lnr != direction.lnr && flags.attributes.test(PlatformAttributes::player_controlled) && player.get_collider().jumpbox.overlaps(bounding_box)) { switch_directions(); }
	if (flags.attributes.test(PlatformAttributes::player_controlled)) {
		state = 2;
		if (player.get_collider().jumpbox.overlaps(bounding_box)) {
			switch (direction.lnr) {
			case LNR::left: state = 3; break;
			case LNR::right: state = 4; break;
			case LNR::neutral: break;
			default: break;
			}
		}
	}
	if (flags.attributes.test(PlatformAttributes::player_activated)) {
		if (player.get_collider().jumpbox.overlaps(bounding_box)) {
			path_position += metrics.speed;
			state = 7;
		} else {
			state = 8;
		}
	} else {
		state = flags.attributes.test(PlatformAttributes::sticky) ? 0 : 1;
		path_position += metrics.speed;
	}
	if (path_position > 1.0f) { path_position = 0.0f; }
	if (path_position < 0.0f) { path_position = 0.0f; }

	sync_components();

	if (old_position != physics.position) {
		flags.state.set(PlatformState::moving);
	} else {
		flags.state.reset(PlatformState::moving);
	}

	counter.update();
	animation.update();
}

void Platform::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	track_shape.setPosition(-cam);
	sprite.setPosition(physics.position - cam);
	auto const u = state * 48;
	auto const v = animation.get_frame() * 112;
	auto lookup = sf::Vector2<int>{u, v} + offset;
	sprite.setTextureRect(sf::IntRect(sf::Vector2<int>(lookup), sf::Vector2<int>(dimensions) / 2));
	if (svc.greyblock_mode()) {
		win.draw(track_shape);
		Collider::render(win, cam);
	} else {
		win.draw(sprite);
	}
}

void Platform::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.transcendent()) { return; }
	if (proj.get_collider().collides_with(bounding_box)) {
		if (!proj.destruction_initiated()) {
			map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_destruction_point() + proj.get_position(), physics.apparent_velocity()));
			if (proj.get_direction().lnr == LNR::neutral) { map.effects.back().rotate(); }
			svc.soundboard.flags.world.set(audio::World::hard_hit);
		}
		proj.destroy(false);
	}
}

void Platform::switch_directions() {
	std::ranges::reverse(track);
	path_position = 1.0f - path_position;
	switch_up.start();
}

} // namespace fornani::world
