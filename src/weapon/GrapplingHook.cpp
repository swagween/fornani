#include "GrapplingHook.hpp"
#include <iostream>

namespace arms {
arms::GrapplingHook::GrapplingHook(int id) : Weapon(id) {

	projectile.physics = components::PhysicsComponent({0.9f, 0.9f}, 1.0f);
	projectile.stats.attractor_force = 0.0525f;
	projectile.attractor = vfx::Attractor(projectile.physics.position, flcolor::goldenrod, projectile.stats.attractor_force);
	projectile.attractor.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.993f, 0.993f}, 1.0f);
	projectile.attractor.collider.physics.maximum_velocity = {60.f, 60.f};

}

} // namespace arms