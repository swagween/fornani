#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/particle/Gravitator.hpp"
#include "fornani/utils/Cooldown.hpp"

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
	Widget(automa::ServiceProvider& svc, sf::Texture& texture, sf::Vector2<int> dim, int index);
	void update(automa::ServiceProvider& svc, int max);
	void render(sf::RenderWindow& win);
	void shake() { shaking.start(); }
	State current_state{};
	vfx::Gravitator gravitator{};
	sf::Vector2<float> position{};
	sf::Vector2<float> draw_position{};
	sf::Vector2<float> origin{};

  private:
	sf::Sprite sprite;
	sf::Vector2<int> dimensions{};
	int maximum{};
	int index{};
	util::Cooldown shaking{100};
};

} // namespace gui
