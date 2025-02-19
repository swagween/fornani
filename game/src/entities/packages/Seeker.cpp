#include "fornani/entities/packages/Seeker.hpp"
#include "fornani/entities/player/Player.hpp"
#include <algorithm>

namespace fornani::entity {

Seeker::Seeker() {
	gravitator = std::make_unique<vfx::Gravitator>(sf::Vector2<float>{}, sf::Color::Transparent, 0.02f);
	gravitator->collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.89f, 0.89f}, 1.0f);
	gravitator->collider.physics.maximum_velocity = sf::Vector2<float>(20.f, 20.f);
}

Seeker::Seeker(float force, float friction) {
	gravitator = std::make_unique<vfx::Gravitator>(sf::Vector2<float>{}, sf::Color::Transparent, force);
	gravitator->collider.physics = components::PhysicsComponent(sf::Vector2<float>{friction, friction}, 1.0f);
	gravitator->collider.physics.maximum_velocity = sf::Vector2<float>(20.f, 20.f);
}

void Seeker::update(automa::ServiceProvider& svc) { gravitator->update(svc); }

void Seeker::seek_player(player::Player& player) { gravitator->set_target_position(player.collider.hurtbox.get_position()); }

} // namespace entity