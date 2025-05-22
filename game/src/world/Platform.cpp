#include "fornani/world/Platform.hpp"
#include <cmath>
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Platform::Platform(automa::ServiceProvider& svc, sf::Vector2<float> position, sf::Vector2<float> dimensions, float extent, std::string_view specifications, float start_point, int style)
	: shape::Collider(dimensions, position), path_position(start_point), sprite{svc.assets.get_texture("platforms")} {

	auto const& in_data = svc.data.platform[specifications];
	if (in_data["sticky"].as_bool()) { flags.attributes.set(PlatformAttributes::sticky); }
	if (in_data["loop"].as_bool()) { flags.attributes.set(PlatformAttributes::loop); }
	if (in_data["repeating"].as_bool()) { flags.attributes.set(PlatformAttributes::repeating); }
	if (in_data["player_activated"].as_bool()) { flags.attributes.set(PlatformAttributes::player_activated); }
	if (in_data["player_controlled"].as_bool()) { flags.attributes.set(PlatformAttributes::player_controlled); }

	metrics.speed = in_data["speed"].as<float>();

	for (auto& point : in_data["track"].array_view()) {
		track.push_back({position.x + (point[0].as<float>() * extent), position.y + (point[1].as<float>() * extent)});
		if (point[0].as<float>() > 0.f) { flags.attributes.set(PlatformAttributes::side_to_side); }
		if (point[1].as<float>() > 0.f) { flags.attributes.set(PlatformAttributes::up_down); }
	}
	track_shape.setPointCount(track.size());
	if (flags.attributes.test(PlatformAttributes::loop)) { track.push_back(track.at(0)); }

	for (std::size_t x = 0; x < track.size() - 1; ++x) {
		path_length += compute_length(track[x + 1] - track[x]);
		track_shape.setPoint(x, track[x] + dimensions * 0.5f);
	}

	track_shape.setFillColor(sf::Color::Transparent);
	track_shape.setOutlineColor(sf::Color(135, 132, 149, 140));
	track_shape.setOutlineThickness(2);

	direction.lr = dir::LR::neutral;
	direction.und = dir::UND::neutral;

	// for collision handling
	if (track.size() > 1) {
		native_direction.lr = flags.attributes.test(PlatformAttributes::side_to_side) ? dir::LR::left : dir::LR::neutral;
		native_direction.und = flags.attributes.test(PlatformAttributes::up_down) ? dir::UND::down : dir::UND::neutral;
	}

	counter.start();

	// set visuals
	animation.set_params({0, 4, 16, -1});
	auto scaled_dim = dimensions / constants::f_cell_size;
	if (scaled_dim.x == 1) { offset = {0, 0}; }
	if (scaled_dim.x == 2) { offset = {32, 0}; }
	if (scaled_dim.x == 3) { offset = {0, 32}; }
	if (scaled_dim.y == 2) { offset = {0, 64}; }
	if (scaled_dim.y == 3) { offset = {0, 128}; }
	switch_up.start();
}

void Platform::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	auto const old_position = physics.position;
	auto edge_start = 0.f;
	player.collider.handle_collider_collision(*this);
	if (player.collider.jumped_into() && physics.velocity.y > 0.f) { player.collider.physics.apply_force(physics.velocity * 8.f); }
	player.on_crush(map);
	for (auto const& enemy : map.enemy_catalog.enemies) { enemy->on_crush(map); }
	switch_up.update();

	// map changes

	// platform changes
	for (auto& breakable : map.breakables) { handle_collider_collision(breakable.get_hurtbox()); }
	for (auto& pushable : map.pushables) {
		// platform should reverse direction upon hitting the sides or top of a pushable
		if (!pushable.collider.jumpbox.overlaps(bounding_box)) {
			handle_collider_collision(pushable.get_hurtbox());
			if (wallslider.overlaps(pushable.get_bounding_box())) { pushable.set_moving(); }
		}
	}
	for (auto& block : map.switch_blocks) {
		if (block.on()) { handle_collider_collision(block.get_hurtbox()); }
	}
	for (auto& platform : map.platforms) {
		if (&platform != this && native_direction.lr != platform.native_direction.lr) { handle_collider_collision(platform.hurtbox); }
	}
	if (flags.state.test(PlatformState::moving)) {
		if (native_direction.lr == dir::LR::left) {
			if (Collider::flags.external_state.consume(shape::ExternalState::horiz_collider_collision) && !switch_up.running()) {
				switch_directions();
				switch_up.start();
			}
		} else {
			if (Collider::flags.external_state.consume(shape::ExternalState::vert_collider_collision) && !switch_up.running()) {
				switch_directions();
				switch_up.start();
			}
		}
	}
	// init direction to oppose player
	direction.lr = player.controller.direction.lr == dir::LR::left ? dir::LR::right : dir::LR::left;

	for (std::size_t x = 0; x < track.size() - 1; ++x) {
		auto const start = track[x];
		auto const end = track[x + 1];
		auto const len = compute_length(end - start);
		if (auto const edge_end = edge_start + (len / path_length); edge_end >= path_position) {
			constexpr auto skip_value{16.f};
			physics.position.x = std::lerp(start.x, end.x, (path_position - edge_start) / (edge_end - edge_start));
			physics.position.y = std::lerp(start.y, end.y, (path_position - edge_start) / (edge_end - edge_start));
			physics.velocity = physics.position - old_position;
			physics.real_velocity = physics.velocity;
			// set direction
			direction.lr = physics.velocity.x > 0.0f ? dir::LR::right : dir::LR::left;
			direction.und = physics.velocity.y > 0.0f ? dir::UND::down : dir::UND::up;

			if (player.collider.jumpbox.overlaps(bounding_box) && !player.collider.perma_grounded() && flags.attributes.test(PlatformAttributes::sticky)) {
				if (!(abs(physics.velocity.x) > skip_value || abs(physics.velocity.y) > skip_value)) { player.collider.physics.forced_momentum = physics.position - old_position; }
			}
			for (auto& pushable : map.pushables) {
				if (pushable.collider.jumpbox.overlaps(bounding_box) && !pushable.collider.perma_grounded() && flags.attributes.test(PlatformAttributes::sticky)) {
					if (!(abs(physics.velocity.x) > skip_value || abs(physics.velocity.y) > skip_value)) { pushable.collider.physics.forced_momentum = physics.position - old_position; }
				}
			}
			break;
		} else {
			edge_start = edge_end;
		}
	}

	if (player.controller.direction.lr != direction.lr && flags.attributes.test(PlatformAttributes::player_controlled) && player.collider.jumpbox.overlaps(bounding_box)) { switch_directions(); }
	if (flags.attributes.test(PlatformAttributes::player_controlled)) {
		state = 2;
		if (player.collider.jumpbox.overlaps(bounding_box)) {
			switch (direction.lr) {
			case dir::LR::left: state = 3; break;
			case dir::LR::right: state = 4; break;
			default: NANI_LOG_WARN(m_logger, "Unknown direction was passed. Did you forget to add a case to the switch?"); break;
			}
		}
	}
	if (flags.attributes.test(PlatformAttributes::player_activated)) {
		if (player.collider.jumpbox.overlaps(bounding_box)) {
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

void Platform::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	track_shape.setPosition(-cam);
	sprite.setPosition(physics.position - cam);
	auto const u = state * 96;
	auto const v = animation.get_frame() * 224;
	auto lookup = sf::Vector2<int>{u, v} + offset;
	sprite.setTextureRect(sf::IntRect(sf::Vector2<int>(lookup), sf::Vector2<int>(dimensions)));
	if (svc.greyblock_mode()) {
		win.draw(track_shape);
		Collider::render(win, cam);
	} else {
		win.draw(sprite);
	}
}

void Platform::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.transcendent()) { return; }
	if (proj.get_bounding_box().overlaps(bounding_box)) {
		if (!proj.destruction_initiated()) {
			map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_destruction_point() + proj.get_position(), physics.velocity * 10.f, proj.effect_type(), 2));
			if (proj.get_direction().lr == dir::LR::neutral) { map.effects.back().rotate(); }
			svc.soundboard.flags.world.set(audio::World::wall_hit);
		}
		proj.destroy(false);
	}
}

void Platform::switch_directions() {
	std::ranges::reverse(track);
	path_position = 1.0f - path_position;
}

} // namespace fornani::world
