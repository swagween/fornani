
#pragma once

#include <SFML/Graphics.hpp>

#include "fornani/io/Logger.hpp"
#include "fornani/setup/ResourceFinder.hpp"

namespace fornani {

namespace gui {
class MiniMap;
}

class WindowMetadata {
  public:
	friend class WindowManager;
	sf::Vector2u display_dimensions{};
	[[nodiscard]] auto i_screen_dimensions() const -> sf::Vector2<int> { return m_screen_dimensions; }
	[[nodiscard]] auto u_screen_dimensions() const -> sf::Vector2u { return sf::Vector2u{static_cast<unsigned>(m_screen_dimensions.x), static_cast<unsigned>(m_screen_dimensions.y)}; }
	[[nodiscard]] auto f_screen_dimensions() const -> sf::Vector2f { return sf::Vector2f{static_cast<float>(m_screen_dimensions.x), static_cast<float>(m_screen_dimensions.y)}; }
	[[nodiscard]] auto f_center_screen() const -> sf::Vector2<float> { return f_screen_dimensions() * 0.5f; }
	[[nodiscard]] auto is_fullscreen() const -> bool { return m_is_fullscreen; }
	[[nodiscard]] auto in_window(sf::Vector2<float> point, sf::Vector2<float> dimensions) const -> bool {
		if (point.x + dimensions.x < 0.f || point.x - dimensions.x > f_screen_dimensions().x) { return false; }
		if (point.y + dimensions.y < 0.f || point.y - dimensions.y > f_screen_dimensions().y) { return false; }
		return true;
	}

  private:
	void set_screen_dimensions(sf::Vector2i to_dim) { m_screen_dimensions = to_dim; }
	void set_fullscreen(bool to) { m_is_fullscreen = to; }
	sf::Vector2i m_screen_dimensions{};
	bool m_is_fullscreen{};
};

class WindowManager {
  public:
	friend class Application;
	friend class Game;
	friend class gui::MiniMap;
	void set();
	void create(std::string const& title, bool fullscreen);
	void restore_view();
	void set_screencap();
	void take_screenshot(data::ResourceFinder& finder);
	sf::RenderWindow& get() { return m_window; }
	[[nodiscard]] auto is_fullscreen() const -> bool { return metadata.is_fullscreen(); }

	WindowMetadata metadata{};

  private:
	void set_view(sf::View view) { m_game_view = view; }
	void set_port(sf::FloatRect port) {
		m_game_port = port;
		m_game_view.setViewport(m_game_view.getViewport());
	}

	sf::View get_view() const { return m_game_view; }
	sf::FloatRect get_viewport() const { return m_game_port; }
	sf::RenderWindow m_window{};
	sf::VideoMode m_mode{};
	sf::FloatRect m_game_port{};
	sf::View m_game_view{};
	sf::Texture screencap{};

	io::Logger m_logger{"windowing"};
};

} // namespace fornani
