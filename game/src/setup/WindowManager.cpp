#include "fornani/setup/WindowManager.hpp"
#include <iostream>

#include <imgui-SFML.h>

namespace fornani {

void WindowManager::set() {
	game_view = sf::View(sf::FloatRect({}, {static_cast<float>(screen_dimensions.x), static_cast<float>(screen_dimensions.y)}));
	// set view and viewport for fullscreen mode
	const auto aspect_ratio = static_cast<float>(aspects.x) / static_cast<float>(aspects.y);
	const auto display_ratio = static_cast<float>(display_dimensions.x) / static_cast<float>(display_dimensions.y);
	const auto letterbox = std::min(display_ratio, aspect_ratio) / std::max(display_ratio, aspect_ratio);
	const auto vertical = display_ratio < aspect_ratio;
	auto resize_ratio = vertical ? sf::Vector2<float>(1.f, letterbox) : sf::Vector2<float>(letterbox, 1.f);
	auto offset = vertical ? sf::Vector2<float>(0.f, (1.f - letterbox) * 0.5f) : sf::Vector2<float>((1.f - letterbox) * 0.5f, 0.f);
	game_port = is_fullscreen ? sf::FloatRect{{offset.x, offset.y}, {resize_ratio.x, resize_ratio.y}} : sf::FloatRect({}, {1.f, 1.f});
	game_view.setViewport(game_port);
	window.setView(game_view);
	if (is_fullscreen) { window.setMouseCursorVisible(false); }

	if (!screencap.resize(window.getSize())) { NANI_LOG_WARN(m_logger, "Failed to resize window."); }
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);
}

void WindowManager::create(const std::string& title, bool const fullscreen) {
	is_fullscreen = fullscreen;
	// set window constants
	screen_dimensions = {aspects.x / 4, aspects.y / 4};
	u_screen_dimensions = {static_cast<uint16_t>(screen_dimensions.x), static_cast<uint16_t>(screen_dimensions.y)};
	display_dimensions = {static_cast<unsigned>(sf::VideoMode::getDesktopMode().size.x), static_cast<unsigned>(sf::VideoMode::getDesktopMode().size.y)};
	mode = fullscreen ? sf::VideoMode(display_dimensions) : sf::VideoMode(u_screen_dimensions);
	if (!mode.isValid() && fullscreen) {
		NANI_LOG_WARN(
			m_logger,
			"Number of valid fullscreen modes: {}\n"
			"Failed to extract a valid fullscreen mode.",
			sf::VideoMode::getFullscreenModes().size());
		mode = sf::VideoMode(u_screen_dimensions);
		is_fullscreen = false;
	}
	is_fullscreen ? window.create(mode, title, sf::State::Fullscreen) : window.create(mode, title, sf::Style::Default & ~sf::Style::Resize);
}


void WindowManager::restore_view() { window.setView(game_view); }

void WindowManager::set_screencap() { if (!screencap.resize(window.getSize())) { NANI_LOG_WARN(m_logger, "Window resize failed!"); } }

} // namespace fornani
