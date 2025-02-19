#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/particle/Gravitator.hpp"

namespace fornani::player {
class Player;
}

namespace fornani::entity {

class Seeker {
  public:
	Seeker();
	Seeker(float force, float friction);
	void update(automa::ServiceProvider& svc);
	void seek_player(player::Player& player);
	void set_position(sf::Vector2<float> const pos) const { gravitator->set_position(pos); }
	void set_force(float const force) const { gravitator->attraction_force = force; }
	void bounce_vert() const { gravitator->collider.physics.velocity.y *= -1.f; }
	void bounce_horiz() const { gravitator->collider.physics.velocity.x *= -1.f; }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return gravitator->collider.bounding_box.get_position(); }
	[[nodiscard]] auto get_velocity() const -> sf::Vector2<float> { return gravitator->collider.physics.velocity; }

  private:
	std::unique_ptr<vfx::Gravitator> gravitator{};
};

} // namespace fornani::entity