#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/particle/Gravitator.hpp"
#include "fornani/utils/Cooldown.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::gui {
enum class State : uint8_t { added, neutral, taken, gone };
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

} // namespace fornani::gui
