
#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>

namespace automa {
struct ServiceProvider;
}

namespace gui {

class Selector {
  public:
	Selector() = default;
	Selector(sf::Vector2<int> dim);
	void update();
	void go_up();
	void go_down();
	void go_left();
	void go_right();
	void set_dimensions(sf::Vector2<int> dim);
	[[nodiscard]] auto get_current_selection() const -> int { return current_selection; }

  private:
	sf::Vector2<int> table_dimensions{};
	int current_selection{};
};

} // namespace gui
