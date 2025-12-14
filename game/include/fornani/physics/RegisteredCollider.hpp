
#pragma once

#include <fornani/physics/CircleCollider.hpp>
#include <fornani/physics/Collider.hpp>

namespace fornani::world {
class Map;
}

namespace fornani::shape {

class RegisteredCollider {
  public:
	RegisteredCollider(world::Map& map, sf::Vector2f dimensions);
	RegisteredCollider(world::Map& map, float radius);

	~RegisteredCollider();
	RegisteredCollider(RegisteredCollider&&) = delete;

	Collider* get() const { return m_collider.value(); }
	Collider& get_reference() const { return *m_collider.value(); }
	CircleCollider* get_circle() const { return m_circle_collider.value(); }
	CircleCollider& get_reference_circle() const { return *m_circle_collider.value(); }

	RegisteredCollider(RegisteredCollider const&) = delete;
	RegisteredCollider& operator=(RegisteredCollider const&) = delete;

  private:
	world::Map* m_map{};
	std::optional<Collider*> m_collider{};
	std::optional<CircleCollider*> m_circle_collider{};
};

} // namespace fornani::shape
