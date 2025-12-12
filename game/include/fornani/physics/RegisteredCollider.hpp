
#pragma once

#include <fornani/physics/Collider.hpp>

namespace fornani::world {
class Map;
}

namespace fornani::shape {

class RegisteredCollider {
  public:
	RegisteredCollider(world::Map& map, sf::Vector2f dimensions);

	~RegisteredCollider();
	RegisteredCollider(RegisteredCollider&&) = delete;

	Collider* get() const { return m_collider; }

	RegisteredCollider(RegisteredCollider const&) = delete;
	RegisteredCollider& operator=(RegisteredCollider const&) = delete;

  private:
	world::Map* m_map{};
	Collider* m_collider{};
};

} // namespace fornani::shape
