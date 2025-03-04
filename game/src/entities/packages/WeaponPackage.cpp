#include "fornani/entities/packages/WeaponPackage.hpp"
#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/service/ServiceProvider.hpp"


namespace fornani::entity {

WeaponPackage::WeaponPackage(automa::ServiceProvider& svc, int id) { weapon = std::make_unique<arms::Weapon>(svc, id, true); }

void WeaponPackage::update(automa::ServiceProvider& svc, world::Map& map, enemy::Enemy& enemy) {
	weapon->update(svc, direction);
	clip_cooldown.update();
	weapon.get()->set_barrel_point (enemy.get_collider().physics.position + barrel_offset);
	sf::Vector2<float> p_pos = {enemy.get_collider().physics.position.x + barrel_offset.x, enemy.get_collider().physics.position.y + barrel_offset.y};
	weapon.get()->set_position(p_pos);
	weapon.get()->set_firing_direction(enemy.directions.actual);
}

void WeaponPackage::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) { weapon->render(svc, win, cam); }

void WeaponPackage::shoot() { weapon->shoot(); }

void WeaponPackage::set_team(arms::Team team) { weapon->set_team(team); }

} // namespace entity