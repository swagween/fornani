
#include "fornani/world/Platform.hpp"
#include <fornani/utils/Math.hpp>
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Platform::Platform(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f position, sf::Vector2f dimensions, float extent, std::string_view specifications, float start_point, int style)
	: Animatable(svc, "platform_" + std::string{map.get_biome_string()}), m_collider{map, dimensions}, path_position(start_point), switch_up{12} {

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

	auto edge_start = 0.f;
	for (std::size_t x = 0; x < track.size() - 1; ++x) {
		auto const start = track[x];
		auto const end = track[x + 1];
		auto const len = (end - start).length();
		if (auto const edge_end = edge_start + (len / path_length); edge_end >= path_position) {
			constexpr auto skip_value{32.f};
			if (flags.attributes.test(PlatformAttributes::ease)) {
				get_collider().physics.position.x = util::ease_in_out(start.x, end.x, (path_position - edge_start) / (edge_end - edge_start));
				get_collider().physics.position.y = util::ease_in_out(start.y, end.y, (path_position - edge_start) / (edge_end - edge_start));
			} else {
				get_collider().physics.position.x = std::lerp(start.x, end.x, (path_position - edge_start) / (edge_end - edge_start));
				get_collider().physics.position.y = std::lerp(start.y, end.y, (path_position - edge_start) / (edge_end - edge_start));
			}
		}
	}
	get_collider().set_trait(shape::ColliderTrait::block);
	get_collider().set_trait(shape::ColliderTrait::platform);
	get_collider().set_attribute(shape::ColliderAttributes::sturdy);
	get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
	get_collider().set_attribute(shape::ColliderAttributes::custom_resolution);
	get_collider().set_exclusion_target(shape::ColliderTrait::particle);
	get_collider().set_resolution_exclusion_target(shape::ColliderTrait::pushable);
	get_collider().set_attribute(shape::ColliderAttributes::crusher);

	for (auto const& t : map.get_entities<Turret>()) {
		if (t->get_world_position() == position) {
			m_turret = t;
			t->set_flag(TurretFlags::platform);
		}
	}
}

void Platform::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	auto edge_start = 0.f;
	switch_up.update();

	m_old_position != get_collider().physics.position ? flags.state.set(PlatformState::moving) : flags.state.reset(PlatformState::moving);
	if (m_turret) { m_turret.value()->Turret::set_position(get_collider().physics.position); }

	// init direction to oppose player
	direction.lnr = player.controller.direction.lnr == LNR::left ? LNR::right : LNR::left;
	if (flags.attributes.test(PlatformAttributes::player_activated)) {
		if (player.get_collider().jumpbox.overlaps(get_collider().bounding_box)) {
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

	for (std::size_t x = 0; x < track.size() - 1; ++x) {
		auto const start = track[x];
		auto const end = track[x + 1];
		auto const len = (end - start).length();
		if (auto const edge_end = edge_start + (len / path_length); edge_end >= path_position) {
			if (flags.attributes.test(PlatformAttributes::ease)) {
				get_collider().physics.position.x = util::ease_in_out(start.x, end.x, (path_position - edge_start) / (edge_end - edge_start));
				get_collider().physics.position.y = util::ease_in_out(start.y, end.y, (path_position - edge_start) / (edge_end - edge_start));
			} else {
				get_collider().physics.position.x = std::lerp(start.x, end.x, (path_position - edge_start) / (edge_end - edge_start));
				get_collider().physics.position.y = std::lerp(start.y, end.y, (path_position - edge_start) / (edge_end - edge_start));
			}
			get_collider().physics.velocity = get_collider().physics.position - m_old_position;
			get_collider().physics.real_velocity = get_collider().physics.velocity;
			// set direction
			direction.lnr = get_collider().physics.velocity.x > 0.0f ? LNR::right : LNR::left;
			direction.und = get_collider().physics.velocity.y > 0.0f ? UND::down : UND::up;

			break;
		} else {
			edge_start = edge_end;
			if (flags.attributes.test(PlatformAttributes::repeating)) {
				path_position = 0.f;
				edge_start = 0.f;
			}
		}
	}
	if (player.controller.direction.lnr != direction.lnr && flags.attributes.test(PlatformAttributes::player_controlled) && player.get_collider().jumpbox.overlaps(get_collider().bounding_box)) { switch_directions(); }
	if (flags.attributes.test(PlatformAttributes::player_controlled)) {
		state = 2;
		if (player.get_collider().jumpbox.overlaps(get_collider().bounding_box)) {
			switch (direction.lnr) {
			case LNR::left: state = 3; break;
			case LNR::right: state = 4; break;
			case LNR::neutral: break;
			default: break;
			}
		}
	}

	counter.update();
	animation.update();
	if (get_velocity().lengthSquared() > constants::tiny_value) {
		auto max_vel = 8.f;
		auto t = std::clamp(get_velocity().lengthSquared() / max_vel, 0.f, 1.f);
		auto pitch = std::lerp(1.f, 2.0f, get_velocity().lengthSquared());
		svc.soundboard.repeat_sound("platform_industrial", m_handle, get_collider().get_center(), pitch);
	}
}

void Platform::post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	constexpr auto skip_value{16.f};
	player.get_collider().handle_collider_collision(get_collider());
	// if (player.get_collider().flags.state.test(shape::State::ceiling_collision)) { player.get_collider().physics.acceleration.y = player.physics_stats.maximum_velocity.y; }
	if (player.get_collider().jumpbox.overlaps(get_collider().bounding_box) && !player.get_collider().perma_grounded() && is_sticky()) {
		auto stuck_left = player.get_collider().has_left_wallslide_collision() && get_collider().physics.velocity.x < 0.f;
		auto stuck_right = player.get_collider().has_right_wallslide_collision() && get_collider().physics.velocity.x > 0.f;
		if (!(stuck_right || stuck_left)) {
			if (!(abs(get_collider().physics.velocity.x) > skip_value || abs(get_collider().physics.velocity.y) > skip_value)) { player.get_collider().physics.forced_momentum = get_collider().physics.position - m_old_position; }
		}
	}
	m_old_position = get_collider().physics.position;
	if (!switch_up.running()) {
		if (native_direction.lnr == LNR::left) {
			if (get_collider().flags.external_state.test(shape::ExternalState::horiz_collider_collision)) {
				switch_directions();
				svc.soundboard.flags.world.set(audio::World::hard_hit);
			}
		} else {
			if (get_collider().flags.external_state.test(shape::ExternalState::vert_collider_collision)) {
				switch_directions();
				svc.soundboard.flags.world.set(audio::World::hard_hit);
			}
		}
	}
}

void Platform::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	track_shape.setPosition(-cam);
	Animatable::set_position(get_collider().physics.position - cam);
	auto const u = state * 48;
	auto const v = animation.get_frame() * 112;
	auto lookup = sf::Vector2<int>{u, v} + offset;
	set_texture_rect(sf::IntRect(sf::Vector2<int>(lookup), sf::Vector2<int>(get_collider().dimensions) / 2));
	if (svc.greyblock_mode()) {
		win.draw(track_shape);
		get_collider().render(win, cam);
	} else {
		win.draw(*this);
	}
}

void Platform::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.transcendent()) { return; }
	if (proj.get_collider().collides_with(get_collider().bounding_box)) {
		if (!proj.destruction_initiated()) {
			map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_destruction_point() + proj.get_position(), get_collider().physics.apparent_velocity()));
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
	NANI_LOG_DEBUG(m_logger, "switched platform directions!");
}

} // namespace fornani::world
