
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/physics/RegisteredCollider.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::vfx {

class Antenna {
  public:
	Antenna(world::Map& map, sf::Vector2f pos, sf::Color col, float agf, sf::Vector2f size = {4.f, 4.f});

	void update(automa::ServiceProvider& svc);
	void add_force(sf::Vector2f force);
	void set_position(sf::Vector2f new_position);
	void set_target_position(sf::Vector2f new_position);
	void demagnetize(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos, int history = 0);
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return get_collider().physics.position; }
	[[nodiscard]] auto get_collider() const -> shape::CircleCollider& { return m_collider.get_reference_circle(); }

	sf::Vector2f dimensions{};

	sf::Color color{};
	sf::RectangleShape box{};
	float attraction_force{};

  protected:
	shape::RegisteredCollider m_collider;
	components::SteeringBehavior steering{};
};

} // namespace fornani::vfx
