
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/physics/CircleCollider.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::vfx {

class Gravitator {

  public:
	Gravitator() = default;
	Gravitator(sf::Vector2f pos, sf::Color col, float agf, sf::Vector2f size = {4.f, 4.f});
	void update(automa::ServiceProvider& svc);
	void add_force(sf::Vector2f force);
	void set_position(sf::Vector2f new_position);
	void set_target_position(sf::Vector2f new_position);
	void demagnetize(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos, int history = 0);
	[[nodiscard]] auto position() const -> sf::Vector2f { return collider.physics.position; }

	shape::CircleCollider collider{4.f};
	sf::Vector2f dimensions{};

	sf::Color color{};
	sf::RectangleShape box{};
	float attraction_force{};

  private:
	components::SteeringBehavior steering{};
};

} // namespace fornani::vfx
