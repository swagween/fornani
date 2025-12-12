
#include <fornani/physics/RegisteredCollider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::shape {

RegisteredCollider::RegisteredCollider(world::Map& map, sf::Vector2f dimensions) : m_map(&map) {
	auto ptr = std::make_unique<Collider>(dimensions);
	m_collider = ptr.get();
	m_map->register_collider(std::move(ptr));
}

RegisteredCollider::~RegisteredCollider() { m_map->unregister_collider(m_collider); }

} // namespace fornani::shape
