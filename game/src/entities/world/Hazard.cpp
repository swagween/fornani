
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/world/Hazard.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

Hazard::Hazard(dj::Json const& in, sf::Vector2f position, sf::Vector2f center, sf::Angle rotation) : m_sensor{in["radius"].as<float>()} {
	auto offset = sf::Vector2f{in["position"][0].as<float>(), in["position"][1].as<float>()};

	// rotate around center
	auto local = offset - center;
	auto rotated = local.rotatedBy(rotation);
	auto final_offset = rotated + center;

	m_sensor.set_position(position + final_offset * constants::f_scale_factor);
}

Hazard::Hazard(sf::Vector2f position, float radius) : m_sensor{radius} { m_sensor.set_position(position); }

void Hazard::update(player::Player& player, world::Map& map) {
	auto player_overlap = m_sensor.within_bounds(player.hurtbox);
	if (map.transition.is(graphics::TransitionState::black)) { player.controller.unrestrict(); }
	if (has_flag_set(HazardFlags::reset) && map.transition.is(graphics::TransitionState::black)) {
		player.set_position(map.last_checkpoint());
		player.get_collider().physics.zero();
		player.controller.prevent_movement();
		player.controller.restrict_movement();
		map.transition.end();
		set_flag(HazardFlags::reset, false);
	}
	if (player_overlap && map.transition.is(graphics::TransitionState::inactive) && !player.invincible() && !player.is_dead()) {
		player.hurt();
		player.freeze_position();
		player.shake_sprite();
		set_flag(HazardFlags::reset);
		map.transition.start();
	}
}

void Hazard::render(sf::RenderWindow& win, sf::Vector2f cam) { m_sensor.render(win, cam); }

} // namespace fornani
