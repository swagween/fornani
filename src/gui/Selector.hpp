
#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include "../utils/Circuit.hpp"

namespace automa {
struct ServiceProvider;
}

namespace gui {

class Selector {
  public:
	Selector() = default;
	Selector(automa::ServiceProvider& svc, sf::Vector2<int> dim);
	void update();
	void render(sf::RenderWindow& win) const;
	void go_up();
	void go_down();
	void go_left();
	void go_right();
	void set_dimensions(sf::Vector2<int> dim);
	void set_position(sf::Vector2<float> pos) { position = pos; }
	[[nodiscard]] auto get_current_selection() const -> int { return current_selection.get(); }

	util::Circuit current_selection{1};

  private:

	automa::ServiceProvider* m_services;

	sf::Vector2<int> table_dimensions{};
	sf::Vector2<float> position{};
	sf::Sprite sprite{};
};

} // namespace gui
