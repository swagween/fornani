
#include <fornani/core/Debug.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/world/Explosion.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::world {

Explosion::Explosion(automa::ServiceProvider& svc, arms::Team team, sf::Vector2f position, float radius, bool stun) : m_sensor{radius}, m_lifetime{8}, m_team{team} {
	m_lifetime.start();
	m_sensor.set_position(position);
	stun ? svc.soundboard.play_sound("stun") : svc.soundboard.flags.arms.set(audio::Arms::frag_grenade);
	set_flag(ExplosionFlags::stun, stun);
}

void Explosion::update(automa::ServiceProvider& svc, player::Player& player, Map& map) {
	m_lifetime.update();
	if (m_sensor.within_bounds(player.hurtbox) && m_team != arms::Team::nani) { has_flag_set(ExplosionFlags::stun) ? player.stun() : player.hurt(); }
	auto exhausted = false;
	for (auto& enemy : map.enemy_catalog.enemies) {
		if (m_sensor.within_bounds(enemy->get_collider().hurtbox) && m_team != enemy->get_team() && !has_flag_set(ExplosionFlags::exhausted)) {
			enemy->hurt(8.f);
			exhausted = true;
		}
	}
	for (auto& breakable : map.breakables) {
		if (m_sensor.within_bounds(breakable->get_collider().bounding_box) && !has_flag_set(ExplosionFlags::exhausted)) {
			breakable->on_smash(svc, map, 2);
			exhausted = true;
		}
	}
	if (exhausted) { set_flag(ExplosionFlags::exhausted); }
}

void Explosion::render(sf::RenderWindow& win, sf::Vector2f cam) {
	if (!debug::is_production()) { m_sensor.render(win, cam); }
}

} // namespace fornani::world
