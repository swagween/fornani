
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/io/Logger.hpp>
#include <memory>

namespace fornani {

class WindowManager {
  public:
	WindowManager(sf::Vector2i const dimensions) : m_screen_dimensions{dimensions} {};
	sf::RenderWindow& get() { return m_window; }
	sf::View get_view() const { return m_game_view; }
	sf::FloatRect get_viewport() const { return m_game_port; }
	void set();
	void create(std::string const& title, bool fullscreen, sf::Vector2i const dimensions);
	void recreate(bool fullscreen);
	void restore_view();
	void set_screencap();
	void set_view(sf::View& to) { m_window.setView(to); }

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
	void try_fullscreen();

  private:
	sf::RenderWindow m_window{};
	sf::Image m_icon;
	sf::View m_game_view{};
	sf::VideoMode m_mode{};
	sf::FloatRect m_game_port{};
	sf::Vector2i m_aspects{3840, 2048};
	sf::Vector2i m_screen_dimensions{};
	sf::Vector2f m_letterbox{1.f, 1.f};
	std::string m_title{};
	bool m_fullscreen{};

	io::Logger m_logger{"windowing"};
};

} // namespace fornani
