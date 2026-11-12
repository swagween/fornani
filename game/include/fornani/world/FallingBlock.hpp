
#pragma once

#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/physics/RegisteredCollider.hpp>

namespace fornani::world {

enum class FallingBlockFlags : std::uint8_t { emerged, crashed };

class FallingBlock : public Animatable, public Flaggable<FallingBlockFlags> {
  public:
	FallingBlock(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f position);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	shape::CircleCollider& get_collider() const { return *m_collider.get_circle(); }

	[[nodiscard]] auto is_gone(world::Map const& map) const -> bool;

  private:
	shape::RegisteredCollider m_collider;
	shape::Shape m_bounding_box;
};

} // namespace fornani::world
