
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/world/Explosion.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::world {

Explosion::Explosion(automa::ServiceProvider& svc, sf::Vector2f position, float radius) : m_sensor{radius}, m_lifetime{8} {
	m_lifetime.start();
	m_sensor.set_position(position);
}

void Explosion::update(automa::ServiceProvider& svc, player::Player& player, Map& map) {
	m_lifetime.update();
	if (m_sensor.within_bounds(player.get_collider().hurtbox)) { player.hurt(); }
}

void Explosion::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (svc.greyblock_mode()) { m_sensor.render(win, cam); }
}

} // namespace fornani::world
