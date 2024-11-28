#include "WeaponPackage.hpp"
#include "../enemy/Enemy.hpp"
#include "../../service/ServiceProvider.hpp"
#include <algorithm>

namespace entity {

WeaponPackage::WeaponPackage(automa::ServiceProvider& svc, std::string_view label, int id) { weapon = std::make_unique<arms::Weapon>(svc, id); }

void WeaponPackage::update(automa::ServiceProvider& svc, world::Map& map, enemy::Enemy& enemy) {
	weapon->update(svc, direction);
	clip_cooldown.update();
	weapon.get()->barrel_point = enemy.get_collider().physics.position + barrel_offset;
	sf::Vector2<float> p_pos = {enemy.get_collider().physics.position.x + barrel_offset.x, enemy.get_collider().physics.position.y + barrel_offset.y};
	weapon.get()->set_position(p_pos);
	weapon.get()->firing_direction = enemy.directions.actual;
}

void WeaponPackage::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) { weapon->render(svc, win, cam); }

void WeaponPackage::shoot() { weapon->shoot(); }

} // namespace entity