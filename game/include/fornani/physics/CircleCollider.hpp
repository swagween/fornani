
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/CircleSensor.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/physics/ICollider.hpp>
#include <fornani/physics/Shape.hpp>
#include <utility>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::shape {

class Collider;

enum class CircleColliderFlags { collided };

class CircleCollider : public ICollider {
  public:
	explicit CircleCollider(float radius);
	void update(automa::ServiceProvider& svc) override;
	void handle_map_collision(world::Map& map) override;
	void handle_collision(ICollider& other) override;
	void handle_collision(Shape& shape, bool soft = false) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam) override;
	void set_position(sf::Vector2f pos) { physics.position = pos; }

	[[nodiscard]] auto collided() const -> bool { return m_flags.test(CircleColliderFlags::collided); }
	[[nodiscard]] auto collides_with(Shape const& shape) const -> bool { return sensor.within_bounds(shape); }
	[[nodiscard]] auto get_collision_result(Shape& shape) const -> sf::Vector2i;
	[[nodiscard]] auto position() const -> sf::Vector2f { return physics.position; }
	[[nodiscard]] auto get_global_center() const -> sf::Vector2f;
	[[nodiscard]] auto get_local_center() const -> sf::Vector2f;
	[[nodiscard]] auto get_radius() const -> float { return sensor.bounds.getRadius(); }
	components::CircleSensor sensor{};

  private:
	util::BitFlags<CircleColliderFlags> m_flags{};
};

} // namespace fornani::shape
