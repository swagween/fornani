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
enum class WidgetState { added, neutral, taken, gone };
class Widget {
  public:
	Widget(automa::ServiceProvider& svc, sf::Texture const& texture, sf::Vector2<int> dim, int index);
	void update(automa::ServiceProvider& svc, int max);
	void render(sf::RenderWindow& win);
	void shake() { shaking.start(); }

	[[nodiscard]] auto get_position() const -> sf::Vector2f { return origin + position; }

	WidgetState current_state{};
	vfx::Gravitator gravitator{};
	sf::Vector2f position{};
	sf::Vector2f draw_position{};
	sf::Vector2f origin{};

  private:
	sf::Sprite sprite;
	sf::Vector2<int> dimensions{};
	int maximum{};
	int index{};
	util::Cooldown shaking{100};
};

} // namespace fornani::gui
