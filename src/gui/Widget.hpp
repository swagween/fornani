#pragma once
#include <SFML/Graphics.hpp>
#include "../particle/Gravitator.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace gui {
enum class State { added, neutral, taken, gone };
class Widget {
  public:
	Widget() = default;
	Widget(automa::ServiceProvider& svc, sf::Vector2<int> dim, int index);
	void update(automa::ServiceProvider& svc, player::Player& player);
	void render(sf::RenderWindow& win);
	State current_state{};
	vfx::Gravitator gravitator{};
	sf::Sprite sprite{};
	sf::Vector2<float> position{};
	sf::Vector2<float> draw_position{};
	sf::Vector2<float> origin{};
  private:
	sf::Vector2<int> dimensions{};
	int maximum{};
	int index{};
};

} // namespace gui
