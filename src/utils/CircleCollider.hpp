#pragma once
#include <cassert>
#include <SFML/Graphics.hpp>
#include "../components/CircleSensor.hpp"
#include "../components/PhysicsComponent.hpp"
#include "Shape.hpp"
#include "Cooldown.hpp"
#include "Counter.hpp"
#include <utility>

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace shape {
enum class CircleColliderFlags { collided };
class CircleCollider {
  public:
	CircleCollider(float radius);
	void update(automa::ServiceProvider& svc);
	void handle_map_collision(world::Map& map);
	void handle_collision(shape::Shape& box);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
	std::pair<size_t, size_t> get_collision_range(world::Map& map);
	[[nodiscard]] auto collided() const -> bool { return flags.test(CircleColliderFlags::collided); }
	[[nodiscard]] auto position() const -> sf::Vector2<float> { return physics.position; }
	components::PhysicsComponent physics{};
  private:
	components::CircleSensor sensor{};
	std::pair<sf::Vector2<float>, sf::Vector2<float>> boundary{};
	sf::Vector2<float> bound{64.f, 64.f};
	util::BitFlags<CircleColliderFlags> flags{};
};
		

} // namespace util