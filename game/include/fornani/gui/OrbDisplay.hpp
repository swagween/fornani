
#pragma once

#include <fornani/graphics/Animatable.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

class OrbDisplay : public Animatable {
  public:
	OrbDisplay(automa::ServiceProvider& svc);
	void update(int amount);
	void render(sf::RenderWindow& win, sf::Vector2f const pos);

  private:
	sf::Text m_amount;
};

} // namespace fornani::gui
