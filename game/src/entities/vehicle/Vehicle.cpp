
#include <fornani/core/Debug.hpp>
#include <fornani/entities/vehicle/Vehicle.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

sf::Vector2i flip_socket(sf::Vector2i socket, sf::Vector2i dimensions) { return {dimensions.x - socket.x, socket.y}; }

Vehicle::Vehicle(automa::ServiceProvider& svc, world::Map& map, std::string_view tag) : Mobile{svc, tag, {svc.data.vehicle[tag]["dimensions"][0].as<int>(), svc.data.vehicle[tag]["dimensions"][1].as<int>()}} {
	auto dimensions = sf::Vector2i{svc.data.vehicle[tag]["dimensions"][0].as<int>(), svc.data.vehicle[tag]["dimensions"][1].as<int>()};
	register_collider(map, {32.f, 32.f});
	get_collider().set_trait(shape::ColliderTrait::prop);
	get_collider().set_attribute(shape::ColliderAttributes::no_collision);
	get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
	get_collider().set_exclusion_target(shape::ColliderTrait::npc);
	p_animatable.push_and_set_animation("basic", anim::Parameters{0, 1, 16, -1});
	p_animatable.center();

	auto const& vehicle = svc.data.vehicle[tag];
	for (auto const& part : vehicle["parts"].as_array()) { m_parts.push_back(VehiclePart(svc, part, dimensions)); }
	m_order = static_cast<DrawOrder>(vehicle["order"].as<int>());
	m_driver_socket = sf::Vector2f{vehicle["driver_socket"][0].as<float>(), vehicle["driver_socket"][1].as<float>()};

	m_flags.set(VehicleFlags::airborne);
}

void Vehicle::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	p_animatable.tick();
	get_collider().physics.set_friction_componentwise({0.99f, 0.99f});
	if (!m_flags.test(VehicleFlags::airborne)) {
		get_collider().set_flag(shape::ColliderFlags::simple, false);
		get_collider().set_attribute(shape::ColliderAttributes::no_collision, false);
		get_collider().set_attribute(shape::ColliderAttributes::no_map_collision, false);
		get_collider().set_flag(shape::ColliderFlags::gravity, true);
		get_collider().physics.gravity = 2.f;
	} else {
		get_collider().set_flag(shape::ColliderFlags::simple);
		m_steering.seek(get_collider().physics, m_target - get_collider().get_local_center(), 0.1f);
	}
	if (m_flags.test(VehicleFlags::idling)) { m_tweak = sf::Vector2f{std::cos(svc.ticker.seconds_passed.count() * 0.1f), std::sin(svc.ticker.seconds_passed.count() * 0.1f) * 10.f}; }

	for (auto& part : m_parts) { part.piece.tick(); }
	if (directions.desired != directions.actual) { request_flip(); }
	Mobile::post_update(svc, map, player);
}

void Vehicle::render(sf::RenderWindow& win, sf::Vector2f cam, DrawOrder order) {
	auto driver_socket = directions.actual.right() ? m_driver_socket : m_driver_socket;
	switch (debug::mode) {
	case debug::PresentationMode::production:
		p_animatable.set_position(get_collider().get_center() + driver_socket - cam);
		if (order == m_order) { win.draw(p_animatable); }
		for (auto& part : m_parts) {
			if (order != part.order) { continue; }
			part.piece.set_position(p_animatable.get_window_position());
			part.piece.set_origin(p_animatable.get_origin());
			part.piece.set_scale(p_animatable.get_scale());
			win.draw(part.piece);
		}
		break;
	case debug::PresentationMode::debug: get_collider().render(win, cam); break;
	case debug::PresentationMode::greyblock: break;
	}
}

void Vehicle::set_position(sf::Vector2f to) {
	m_target = to;
	get_collider().physics.position = to;
}

void Vehicle::set_idle(bool to) {
	if (to) {
		if (!m_flags.test(VehicleFlags::idling)) { m_target = get_collider().physics.position; }
		m_flags.set(VehicleFlags::idling);
	} else {
		m_flags.reset(VehicleFlags::idling);
		m_tweak = {};
	}
}

VehiclePart::VehiclePart(automa::ServiceProvider& svc, dj::Json const& in, sf::Vector2i dimensions) : piece{svc, in["tag"].as_string(), dimensions}, order{static_cast<DrawOrder>(in["order"].as<int>())} {
	if (in["animations"].is_array()) {
		for (auto const& animation : in["animations"].as_array()) {
			auto const& parameters = animation["parameters"].as_array();
			anim::Parameters params{};
			params.lookup = parameters[0].as<int>();
			params.duration = parameters[1].as<int>();
			params.framerate = parameters[2].as<int>();
			params.num_loops = parameters[3].as<int>();
			params.repeat_last_frame = parameters[4].as_bool();
			piece.push_and_set_animation(animation["label"].as_string(), params);
		}
	} else {
		piece.push_and_set_animation("basic", anim::Parameters{0, 1, 16, -1});
	}
}

} // namespace fornani
