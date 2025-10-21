
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/gui/HealthBar.hpp>
#include <string_view>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

class BossHealth : public HealthBar {
  public:
	BossHealth(automa::ServiceProvider& svc, std::string_view label);
	void update(float const percentage);
	void render(sf::RenderWindow& win);

  private:
	sf::Text m_label;
};

} // namespace fornani::gui
