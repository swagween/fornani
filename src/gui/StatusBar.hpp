#pragma once
#include <SFML/Graphics.hpp>
#include "../particle/Gravitator.hpp"

namespace automa {
struct ServiceProvider;
}

namespace gui {
enum class BarState { full, empty };
class StatusBar {
  public:
	StatusBar() = default;
	StatusBar(automa::ServiceProvider& svc, sf::Vector2<int> dim = {8, 2}, float size = 600.f);
	void update(automa::ServiceProvider& svc, float current);
	void render(sf::RenderWindow& win);
	BarState current_state{};
	vfx::Gravitator gravitator{};
	sf::Vector2<float> position{};
	sf::Vector2<float> draw_position{};
	sf::Vector2<float> origin{};
	[[nodiscard]] auto empty() const -> bool { return current_state == BarState::empty; }

  private:
	sf::Vector2<int> dimensions{};
	float maximum{};
	float size{};
	struct {
		sf::RectangleShape gone{};
		sf::RectangleShape taken{};
		sf::RectangleShape filled{};
	} debug_rects{};
};

} // namespace gui