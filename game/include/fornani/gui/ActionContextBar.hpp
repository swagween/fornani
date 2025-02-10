#pragma once

#include "fornani/setup/ControllerMap.hpp"
#include "fornani/utils/RichText.hpp"

#include <SFML/Graphics.hpp>

namespace fornani::automa {
struct ServiceProvider;
} // namespace fornani::automa

namespace sf {
class RenderWindow;
} // namespace sf

namespace fornani::gui {

class ActionContextBar {
  public:
	ActionContextBar() = default;
	explicit ActionContextBar(automa::ServiceProvider& svc);

	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);

  private:
	util::RichText text{};
};

} // namespace fornani::gui