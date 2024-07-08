
#pragma once
#include "Console.hpp"
#include "Selector.hpp"

namespace player {
class Player;
}

namespace gui {

class MiniMap {
  public:
	MiniMap() = default;
	MiniMap(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win);
  private:
	sf::View view{};
	sf::RectangleShape background{};
};

} // namespace gui
