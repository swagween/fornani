
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

#include "fornani/io/Logger.hpp"

namespace fornani {

class WindowManager {
  public:
	sf::RenderWindow& get() { return *m_window.get(); }
	sf::View get_view() const { return game_view; }
	sf::FloatRect get_viewport() const { return game_port; }
	void set();
	void create(std::string const& title, bool fullscreen);
	void restore_view();
	void set_screencap();
	[[nodiscard]] auto i_screen_dimensions() const -> sf::Vector2<int> { return m_screen_dimensions; }
	[[nodiscard]] auto f_screen_dimensions() const -> sf::Vector2<float> { return sf::Vector2<float>{static_cast<float>(m_screen_dimensions.x), static_cast<float>(m_screen_dimensions.y)}; }
	[[nodiscard]] auto f_center_screen() const -> sf::Vector2<float> { return f_screen_dimensions() * 0.5f; }
	[[nodiscard]] auto fullscreen() const -> bool { return is_fullscreen; }
	[[nodiscard]] auto in_window(sf::Vector2<float> point, sf::Vector2<float> dimensions) const -> bool {
		if (point.x + dimensions.x < 0.f || point.x - dimensions.x > f_screen_dimensions().x) { return false; }
		if (point.y + dimensions.y < 0.f || point.y - dimensions.y > f_screen_dimensions().y) { return false; }
		return true;
	}

	sf::Texture screencap{};
	sf::Vector2u u_screen_dimensions{};
	sf::Vector2u display_dimensions{};

  private:
	std::unique_ptr<sf::RenderWindow> m_window{};
	sf::Image m_icon;
	sf::View game_view{};
	sf::VideoMode mode{};
	sf::FloatRect game_port{};
	sf::Vector2<int> aspects{3840, 2048};
	sf::Vector2<int> m_screen_dimensions{};
	bool is_fullscreen{};

	io::Logger m_logger{"windowing"};
};

} // namespace fornani
