
#include <fornani/graphics/Colors.hpp>
#include <fornani/setup/WindowManager.hpp>
#include <fornani/utils/Constants.hpp>
#include "app/app_icon_data.hpp" // embedded icon data
#include <imgui-SFML.h>

namespace fornani {

void WindowManager::set() {
	m_game_view = sf::View(sf::FloatRect({}, f_screen_dimensions()));
	// set view and viewport for fullscreen mode
	auto const aspect_ratio = static_cast<float>(m_aspects.x) / static_cast<float>(m_aspects.y);
	auto const display_ratio = get_f_display_dimensions().x / get_f_display_dimensions().y;
	auto const letterbox = std::min(display_ratio, aspect_ratio) / std::max(display_ratio, aspect_ratio);
	auto const vertical = display_ratio < aspect_ratio;
	if (letterbox != 0.f) { m_letterbox = vertical ? sf::Vector2f(1.f, letterbox) : sf::Vector2f(letterbox, 1.f); }
	auto offset = vertical ? sf::Vector2f(0.f, (1.f - letterbox) * 0.5f) : sf::Vector2f((1.f - letterbox) * 0.5f, 0.f);
	if (!m_fullscreen) { m_letterbox = {1.f, 1.f}; }
	m_game_port = m_fullscreen ? sf::FloatRect{{offset.x, offset.y}, {m_letterbox.x, m_letterbox.y}} : sf::FloatRect({}, {1.f, 1.f});
	m_game_view.setViewport(m_game_port);
	m_window.setView(m_game_view);
	if (m_fullscreen) { m_window.setMouseCursorVisible(false); }

	if (!screencap.resize(m_window.getSize())) { NANI_LOG_WARN(m_logger, "Failed to resize screencap!"); }
	m_window.setVerticalSyncEnabled(true);
	m_window.setFramerateLimit(default_framerate_limit_v);
	m_window.setKeyRepeatEnabled(false);

	NANI_LOG_INFO(m_logger, "Letterbox size:		[{}, {}]", m_letterbox.x, m_letterbox.y);
	NANI_LOG_INFO(m_logger, "Game Port size:		[{}, {}]", offset.x, offset.y);
	NANI_LOG_INFO(m_logger, "Display Dimensions:	[{}, {}]", get_display_dimensions().x, get_display_dimensions().y);
	NANI_LOG_INFO(m_logger, "Screen Dimensions:		[{}, {}]", m_screen_dimensions.x, m_screen_dimensions.y);
	NANI_LOG_INFO(m_logger, "Viewport Position:		[{}, {}]", m_game_port.position.x, m_game_port.position.y);
	NANI_LOG_INFO(m_logger, "Viewport Dimensions:	[{}, {}]", m_game_port.size.x, m_game_port.size.y);
	NANI_LOG_INFO(m_logger, "View Dimensions:		[{}, {}]", m_window.getView().getSize().x, m_window.getView().getSize().y);
	NANI_LOG_INFO(m_logger, "Window Dimensions:		[{}, {}]", m_window.getSize().x, m_window.getSize().y);
	NANI_LOG_INFO(m_logger, "Bounds Dimensions:		[{}, {}]", get_bounds().x, get_bounds().y);
}

void WindowManager::create(std::string const& title, bool const fullscreen, sf::Vector2i const dimensions) {
	m_fullscreen = fullscreen;
	m_title = title;
	// set window constants
	m_screen_dimensions = dimensions;
	try_fullscreen();
	// set app icon
	if (!m_icon.loadFromMemory(generated::icon_png.data(), generated::icon_png.size())) { NANI_LOG_WARN(m_logger, "Failed to load application icon.\n"); };

	m_window = sf::RenderWindow(m_mode, title, m_fullscreen ? sf::Style::None : sf::Style::Default & ~sf::Style::Resize, m_fullscreen ? sf::State::Windowed : sf::State::Windowed);
	m_window.setVisible(false);
	while (std::optional const event = m_window.pollEvent()) {}
	m_window.clear(colors::ui_black);
	m_window.display();
	m_window.requestFocus();
	m_window.setIcon(m_icon.getSize(), m_icon.getPixelsPtr());
}

void WindowManager::recreate(bool fullscreen) {
	m_fullscreen = fullscreen;
	try_fullscreen();
	m_window.create(m_mode, m_title, fullscreen ? sf::Style::None : sf::Style::Default & ~sf::Style::Resize, m_fullscreen ? sf::State::Windowed : sf::State::Windowed);
	set();
}

void WindowManager::restore_view() { m_window.setView(m_game_view); }

void WindowManager::set_screencap() {
	if (!screencap.resize(m_window.getSize())) { NANI_LOG_WARN(m_logger, "Window resize failed!"); }
}

[[nodiscard]] auto WindowManager::in_window(sf::Vector2f point, sf::Vector2f dimensions) const -> bool {
	if (point.x + dimensions.x < 0.f || point.x - dimensions.x > f_screen_dimensions().x) { return false; }
	if (point.y + dimensions.y < 0.f || point.y - dimensions.y > f_screen_dimensions().y) { return false; }
	return true;
}

[[nodiscard]] auto WindowManager::get_scale() const -> float { return m_fullscreen ? get_f_display_dimensions().x / m_screen_dimensions.x : 1.f; }

void WindowManager::try_fullscreen() {
	m_mode = m_fullscreen ? sf::VideoMode(get_display_dimensions()) : sf::VideoMode(u_screen_dimensions());
	if (!m_mode.isValid() && m_fullscreen) {
		NANI_LOG_WARN(m_logger,
					  "Number of valid fullscreen modes: {}\n"
					  "Failed to extract a valid fullscreen mode.",
					  sf::VideoMode::getFullscreenModes().size());
		m_mode = sf::VideoMode(u_screen_dimensions());
		m_fullscreen = false;
	}
}

[[nodiscard]] auto WindowManager::get_bounds() const -> sf::Vector2f { return f_screen_dimensions().componentWiseDiv(m_letterbox); }

} // namespace fornani
