
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>
#include <string_view>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

class BossHealth {
  public:
	BossHealth(automa::ServiceProvider& svc, std::string_view label);
	void update(float const percentage);
	void render(sf::RenderWindow& win);

  private:
	sf::RectangleShape m_backdrop{};
	sf::RectangleShape m_taken{};
	sf::RectangleShape m_current{};
	sf::Text m_label;
	sf::Vector2f m_bar_size{};
	components::SteeringBehavior m_steering{};
	components::PhysicsComponent m_physics{};
};

} // namespace fornani::gui
