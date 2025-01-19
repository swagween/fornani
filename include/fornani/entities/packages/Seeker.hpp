#pragma once
#include <../../../../out/vs22-internal/_deps/sfml-src/include/SFML/Graphics.hpp>
#include "../../../../src/particle/Gravitator.hpp"

namespace player {
class Player;
}

namespace entity {

class Seeker {
  public:
	Seeker();
	Seeker(float force, float friction);
	void update(automa::ServiceProvider& svc);
	void seek_player(player::Player& player);
	void set_position(sf::Vector2<float> pos) { gravitator->set_position(pos); }
	void set_force(float force) { gravitator->attraction_force = force; }
	void bounce_vert() { gravitator->collider.physics.velocity.y *= -1.f; }
	void bounce_horiz() { gravitator->collider.physics.velocity.x *= -1.f; }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return gravitator->collider.bounding_box.position; }
	[[nodiscard]] auto get_velocity() const -> sf::Vector2<float> { return gravitator->collider.physics.velocity; }

  private:
	std::unique_ptr<vfx::Gravitator> gravitator{};
};

} // namespace entity