

#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/SteeringComponent.hpp>
#include <fornani/utils/Polymorphic.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

class HealthBar : public Polymorphic {
  public:
	HealthBar(automa::ServiceProvider& svc, sf::Color color);
	virtual void update(float const percentage, sf::Vector2f position, bool centered = false);
	virtual void render(sf::RenderWindow& win, sf::Vector2f cam, bool window_fixed = false);
	void center();

  protected:
	sf::RectangleShape p_backdrop{};
	sf::RectangleShape p_taken{};
	sf::RectangleShape p_current{};
	sf::Vector2f p_bar_size{};
	sf::Vector2f m_world_position{};
	components::SteeringComponent p_steering{};
};

} // namespace fornani::gui
