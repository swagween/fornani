#pragma once
#include <SFML/Graphics.hpp>
#include "../particle/Gravitator.hpp"
#include "../utils/Cooldown.hpp"
#include "Widget.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace entity {
class Health;
}

namespace gui {
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
