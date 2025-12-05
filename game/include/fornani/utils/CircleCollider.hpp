#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/CircleSensor.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/utils/Shape.hpp>

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
	explicit CircleCollider(float radius);
	void update(automa::ServiceProvider& svc, bool simple = false);
	void handle_map_collision(world::Map& map);
	void handle_collision(Shape& shape, bool soft = false);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f pos) { physics.position = pos; }

	[[nodiscard]] auto collided() const -> bool { return flags.test(CircleColliderFlags::collided); }
	[[nodiscard]] auto collides_with(Shape const& shape) const -> bool { return sensor.within_bounds(shape); }
	[[nodiscard]] auto get_collision_result(Shape& shape) const -> sf::Vector2i;
	[[nodiscard]] auto position() const -> sf::Vector2f { return physics.position; }
	[[nodiscard]] auto get_global_center() const -> sf::Vector2f;
	[[nodiscard]] auto get_local_center() const -> sf::Vector2f;
	[[nodiscard]] auto get_radius() const -> float { return sensor.bounds.getRadius(); }
	[[nodiscard]] auto boundary_width() const -> float { return boundary.second.x - boundary.first.x; }

	components::PhysicsComponent physics{};
	std::pair<sf::Vector2f, sf::Vector2f> boundary{};
	components::CircleSensor sensor{};

  private:
	sf::Vector2f bound{64.f, 64.f};
	util::BitFlags<CircleColliderFlags> flags{};
};

} // namespace fornani::shape
