#pragma once
#include <SFML/Graphics.hpp>

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace gui {
enum class IconType { gamepad1, keyboard };
class ActionControlIcon {
  public:
	ActionControlIcon(automa::ServiceProvider& svc, IconType type, sf::Vector2<int> lookup);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam = {});
	void set_position(sf::Vector2<float> pos);

  private:
	sf::Sprite sprite{};
	sf::Vector2<int> dimensions{36, 36};
	sf::Vector2<float> position{};
};

} // namespace gui
