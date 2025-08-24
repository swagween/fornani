
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/io/Logger.hpp>
#include <memory>

namespace fornani {

class WindowManager {
  public:
	sf::RenderWindow& get() { return *m_window.get(); }
	sf::View get_view() const { return game_view; }
	sf::FloatRect get_viewport() const { return game_port; }
	void set();
	void create(std::string const& title, bool fullscreen, sf::Vector2i const dimensions);
	void restore_view();
	void set_screencap();
	void set_view(sf::View& to) { m_window->setView(to); }

	[[nodiscard]] auto i_screen_dimensions() const -> sf::Vector2i { return m_screen_dimensions; }
	[[nodiscard]] auto f_screen_dimensions() const -> sf::Vector2f { return sf::Vector2f{m_screen_dimensions}; }
	[[nodiscard]] auto u_screen_dimensions() const -> sf::Vector2u { return sf::Vector2u{m_screen_dimensions}; }
	[[nodiscard]] auto get_display_dimensions() const -> sf::Vector2u { return sf::VideoMode::getDesktopMode().size; }
	[[nodiscard]] auto get_f_display_dimensions() const -> sf::Vector2f { return sf::Vector2f{sf::VideoMode::getDesktopMode().size}; }
	[[nodiscard]] auto get_letterbox() const -> sf::Vector2f { return m_letterbox; }
	[[nodiscard]] auto get_bounds() const -> sf::Vector2f;
	[[nodiscard]] auto f_center_screen() const -> sf::Vector2f { return f_screen_dimensions() * 0.5f; }
	[[nodiscard]] auto is_fullscreen() const -> bool { return m_fullscreen; }
	[[nodiscard]] auto in_window(sf::Vector2f point, sf::Vector2f dimensions) const -> bool;
	[[nodiscard]] auto get_scale() const -> float;

	sf::Texture screencap{};

  private:
	std::unique_ptr<sf::RenderWindow> m_window{};
	sf::Image m_icon;
	sf::View game_view{};
	sf::VideoMode mode{};
	sf::FloatRect game_port{};
	sf::Vector2i aspects{3840, 2048};
	sf::Vector2i m_screen_dimensions{};
	sf::Vector2f m_letterbox{1.f, 1.f};
	bool m_fullscreen{};

	io::Logger m_logger{"windowing"};
};

} // namespace fornani
