
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/world/Explosion.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::world {

Explosion::Explosion(automa::ServiceProvider& svc, arms::Team team, sf::Vector2f position, float radius) : m_sensor{radius}, m_lifetime{8}, m_team{team} {
	m_lifetime.start();
	m_sensor.set_position(position);
	svc.soundboard.flags.arms.set(audio::Arms::frag_grenade);
}

void Explosion::update(automa::ServiceProvider& svc, player::Player& player, Map& map) {
	m_lifetime.update();
	if (m_sensor.within_bounds(player.get_collider().hurtbox) && m_team != arms::Team::nani) { player.hurt(); }
	auto exhausted = false;
	for (auto& enemy : map.enemy_catalog.enemies) {
		if (m_sensor.within_bounds(enemy->get_collider().hurtbox) && m_team != enemy->get_team() && !has_flag_set(ExplosionFlags::exhausted)) {
			enemy->hurt(8.f);
			exhausted = true;
		}
	}
	if (exhausted) { set_flag(ExplosionFlags::exhausted); }
}

void Explosion::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (svc.greyblock_mode()) { m_sensor.render(win, cam); }
}

} // namespace fornani::world
