#pragma once

#include "fornani/setup/ControllerMap.hpp"
#include "fornani/utils/RichText.hpp"

#include <SFML/Graphics.hpp>

#include <unordered_set>

namespace automa {
struct ServiceProvider;
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
	util::RichText text{};
};

} // namespace gui