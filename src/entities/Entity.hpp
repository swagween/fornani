#pragma once

#include <SFML/Graphics.hpp>

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace entity {

class Entity {
  public:
	Entity() = default;
	Entity(automa::ServiceProvider& svc){};
	virtual void update(automa::ServiceProvider& svc, world::Map& map) = 0;
	virtual void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) = 0;
	sf::RectangleShape drawbox{}; // for debug

  protected:
	sf::Vector2<float> dimensions{};
	sf::Vector2<float> sprite_offset{};
	sf::Vector2<int> sprite_dimensions{};
	sf::Vector2<int> spritesheet_dimensions{};
	sf::Sprite sprite{};
};

} // namespace entity