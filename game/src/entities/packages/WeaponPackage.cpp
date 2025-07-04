#include "fornani/entities/packages/WeaponPackage.hpp"
#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::entity {

WeaponPackage::WeaponPackage(automa::ServiceProvider& svc, int id) { weapon = std::make_unique<arms::Weapon>(svc, id, true); }

void WeaponPackage::update(automa::ServiceProvider& svc, world::Map& map, enemy::Enemy& enemy) {
	weapon->update(svc, direction);
	clip_cooldown.update();
	weapon.get()->set_barrel_point(enemy.get_collider().get_center() + barrel_offset);
	weapon.get()->set_firing_direction(enemy.get_actual_direction());
}

void WeaponPackage::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) { weapon->render(svc, win, cam); }

void WeaponPackage::shoot(automa::ServiceProvider& svc, world::Map& map) { weapon->shoot(svc, map); }

void WeaponPackage::set_team(arms::Team team) { weapon->set_team(team); }

} // namespace fornani::entity
