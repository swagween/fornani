#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/particle/Gravitator.hpp"
#include "Widget.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::entity {
class Health;
}

namespace fornani::gui {
class WidgetBar {
  public:
	void set(automa::ServiceProvider& svc, int amount, sf::Vector2<int> dimensions, sf::Texture& texture, sf::Vector2<float> origin, float pad = 2.f);
	void update(automa::ServiceProvider& svc, entity::Health& health, bool shake = false);
	void render(sf::RenderWindow& win);

  private:
	std::vector<Widget> widgets{};
	sf::Vector2<float> position{};
};
} // namespace gui
