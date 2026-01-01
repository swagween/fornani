
#pragma once

#include <fornani/io/Logger.hpp>
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

	void set_tag(std::string_view tag) { m_tag = tag; }

	[[nodiscard]] auto get_tag() const -> std::optional<std::string> { return m_tag; }

  private:
	world::Map* m_map{};
	std::optional<Collider*> m_collider{};
	std::optional<CircleCollider*> m_circle_collider{};

	std::optional<std::string> m_tag{};
	io::Logger m_logger{"Collider"};
};

} // namespace fornani::shape
