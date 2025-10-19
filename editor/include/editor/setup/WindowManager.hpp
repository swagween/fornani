
#pragma once

#include <SFML/Graphics.hpp>

#include "fornani/io/Logger.hpp"

namespace pi {

class WindowManager {
  public:
	sf::RenderWindow& get() { return window; }
	sf::View get_view() const { return game_view; }
	sf::FloatRect get_viewport() const { return game_port; }
	void set();
	void create(std::string title, bool const fullscreen);
	void restore_view();
	void set_screencap();
	void resize();
	[[nodiscard]] auto fullscreen() const -> bool { return is_fullscreen; }
	[[nodiscard]] auto f_screen_dimensions() const -> sf::Vector2f { return sf::Vector2f{static_cast<float>(dimensions.current.x), static_cast<float>(dimensions.current.y)}; }
	[[nodiscard]] auto f_center_screen() const -> sf::Vector2f { return f_screen_dimensions() * 0.5f; }

	sf::Texture screencap{};

  private:
	sf::RenderWindow window{};
	sf::View game_view{};
	sf::VideoMode mode{};
	sf::FloatRect game_port{};
	bool is_fullscreen{};
	struct {
		sf::Vector2u preset{1920, 1080};
		sf::Vector2u display{};
		sf::Vector2u current{};
	} dimensions{};

	fornani::io::Logger m_logger{ "pioneer" };
};

} // namespace pi
