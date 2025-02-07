#pragma once
#include <cassert>
#include <SFML/Graphics.hpp>
#include "fornani/components/CircleSensor.hpp"
#include "fornani/components/PhysicsComponent.hpp"
#include "Shape.hpp"
#include "Cooldown.hpp"
#include "fornani/utils/Counter.hpp"
#include <utility>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::shape {
enum class CircleColliderFlags { collided };
class CircleCollider {
  public:
	CircleCollider(float radius);
	void update(automa::ServiceProvider& svc);
	void handle_map_collision(world::Map& map);
	void handle_collision(shape::Shape& shape, bool soft = false);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_position(sf::Vector2<float> pos) { physics.position = pos; }
	[[nodiscard]] auto collided() const -> bool { return flags.test(CircleColliderFlags::collided); }
	[[nodiscard]] auto collides_with(shape::Shape& shape) const -> bool { return sensor.within_bounds(shape); }
	[[nodiscard]] auto position() const -> sf::Vector2<float> { return physics.position; }
	[[nodiscard]] auto boundary_width() const -> float { return boundary.second.x - boundary.first.x; }
	components::PhysicsComponent physics{};
	std::pair<sf::Vector2<float>, sf::Vector2<float>> boundary{};
	components::CircleSensor sensor{};
  private:
	sf::Vector2<float> bound{64.f, 64.f};
	util::BitFlags<CircleColliderFlags> flags{};
};
		

} // namespace util