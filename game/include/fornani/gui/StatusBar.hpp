#pragma once
#include <SFML/Graphics.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/entities/packages/Health.hpp>
#include <fornani/graphics/Colors.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

class StatusBar {
  public:
	explicit StatusBar(automa::ServiceProvider& svc, sf::Vector2f dim = {8.f, 2.f}, std::vector<sf::Color> colors = {colors::red, colors::goldenrod, colors::navy_blue}, bool centered = true);
	void update(automa::ServiceProvider& svc, sf::Vector2f position, entity::Health& status, bool ease = false);
	void update(automa::ServiceProvider& svc, sf::Vector2f position, float fraction, float taken = 0.f, bool ease = false);
	void render(sf::RenderWindow& win);
	void set_dimensions(sf::Vector2f dim) { m_dimensions = dim; }
	[[nodiscard]] auto get_dimensions() const -> sf::Vector2f { return m_dimensions; }

  private:
	components::PhysicsComponent m_physics{};
	components::SteeringBehavior m_steering{};
	sf::Vector2f m_dimensions{};
	float m_unit_size{};
	struct {
		sf::RectangleShape gone{};
		sf::RectangleShape taken{};
		sf::RectangleShape filled{};
	} m_rects{};
};

} // namespace fornani::gui
