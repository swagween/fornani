#pragma once

#include <SFML/Graphics/Text.hpp>

namespace automa {
class ServiceProvider;
}

namespace sf {
class RenderWindow;
}

namespace gui {

class ActionContextBar {
  public:
	ActionContextBar() = default;
	ActionContextBar(automa::ServiceProvider& svc);

	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);

  private:
	sf::Text text{};
};

} // namespace gui