#pragma once
#include <SFML/Graphics.hpp>
#include "../particle/Gravitator.hpp"
#include "../utils/Cooldown.hpp"

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
	void update(automa::ServiceProvider& svc, int max);
	void render(sf::RenderWindow& win);
	void shake() { shaking.start(); }
	State current_state{};
	vfx::Gravitator gravitator{};
	sf::Sprite sprite;
	sf::Vector2<float> position{};
	sf::Vector2<float> draw_position{};
	sf::Vector2<float> origin{};
  private:
	sf::Vector2<int> dimensions{};
	int maximum{};
	int index{};
	util::Cooldown shaking{100};
};

} // namespace gui
