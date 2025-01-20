
#pragma once

#include <../../../out/vs22-internal/_deps/sfml-src/include/SFML/Graphics.hpp>

namespace fornani {

class WindowManager {
  public:
	sf::RenderWindow& get() { return window; }
	sf::View get_view() { return game_view; }
	sf::FloatRect get_viewport() const { return game_port; }
	void set();
	void create(std::string title, bool const fullscreen);
	void restore_view();
	void set_screencap();
	[[nodiscard]] auto fullscreen() const -> bool { return is_fullscreen; }

	sf::Texture screencap{};
	sf::Vector2<int> screen_dimensions{};
	sf::Vector2u u_screen_dimensions{};
	sf::Vector2u display_dimensions{};

  private:
	sf::RenderWindow window{};
	sf::View game_view{};
	sf::VideoMode mode{};
	sf::FloatRect game_port{};
	sf::Vector2<int> aspects{3840, 2048};
	bool is_fullscreen{};
};

} // namespace fornani
