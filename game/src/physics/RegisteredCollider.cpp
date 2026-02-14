
#include <fornani/physics/RegisteredCollider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::shape {

RegisteredCollider::RegisteredCollider(world::Map& map, sf::Vector2f dimensions) : m_map(&map) {
	auto ptr = std::make_unique<Collider>(dimensions);
	m_collider = ptr.get();
	ptr.get()->set_flag(ColliderFlags::registered);
	m_map->register_collider(std::move(ptr));
}

RegisteredCollider::RegisteredCollider(world::Map& map, float radius) : m_map(&map) {
	auto ptr = std::make_unique<CircleCollider>(radius);
	m_circle_collider = ptr.get();
	ptr.get()->set_flag(ColliderFlags::registered);
	m_map->register_collider(std::move(ptr));
}

RegisteredCollider::~RegisteredCollider() {
	if (m_tag) { NANI_LOG_DEBUG(m_logger, "Deleting Collider with tag {}", m_tag.value()); }
	if (m_collider.has_value()) { m_map->unregister_collider(m_collider.value()); }
	if (m_circle_collider.has_value()) { m_map->unregister_collider(m_circle_collider.value()); }
}

} // namespace fornani::shape
