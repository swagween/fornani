#pragma once
#include <SFML/Graphics.hpp>
#include "Cooldown.hpp"
#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::util {

class NineSlice {
  public:
	NineSlice(automa::ServiceProvider& svc, sf::Texture& tex, sf::Vector2i edge, sf::Vector2i corner);
	void render(sf::RenderWindow& win, sf::Vector2f cam = {});
	void target_position(sf::Vector2f to_position, float strength = 0.001f) { m_steering.target(m_physics, to_position, strength); }
	void set_position(sf::Vector2f to_position) { m_physics.position = to_position; }
	void set_dimensions(sf::Vector2f to_dimensions) { m_dimensions = to_dimensions; }
	[[nodiscard]] auto get_local_center() const -> sf::Vector2f { return m_dimensions * 0.5f; }
	[[nodiscard]] auto get_global_center() const -> sf::Vector2f { return m_physics.position + m_dimensions * 0.5f; }
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_physics.position; }

  private:
	components::PhysicsComponent m_physics{};
	components::SteeringBehavior m_steering{};
	sf::Sprite m_sprite;
	sf::Vector2i m_corner_dimensions{};
	sf::Vector2i m_edge_dimensions{};
	sf::Vector2f m_dimensions{};
};

} // namespace fornani::util
