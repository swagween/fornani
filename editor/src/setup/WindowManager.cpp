#include "editor/setup/WindowManager.hpp"

namespace pi {

void WindowManager::set() {

	game_view = sf::View(sf::FloatRect({}, {static_cast<float>(dimensions.current.x), static_cast<float>(dimensions.current.y)}));

	// set view and veiwport for fullscreen mode
	game_port = sf::FloatRect{{0.f, 0.f}, {1.f, 1.f}};
	game_view.setViewport(game_port);
	window.setView(game_view);

	if (!screencap.resize(window.getSize())) { NANI_LOG_WARN(m_logger, "Failed to resize screencap."); }
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);
}

void WindowManager::create(std::string title, bool const fullscreen) {
	is_fullscreen = fullscreen;
	// set window constants
	dimensions.display = sf::Vector2u{sf::VideoMode::getDesktopMode().size};
	mode = is_fullscreen ? sf::VideoMode(dimensions.display) : sf::VideoMode(dimensions.preset);
	if (!mode.isValid()) {
		NANI_LOG_INFO(m_logger, "Number of valid fullscreen modes: {}", mode.getFullscreenModes().size());
		NANI_LOG_WARN(m_logger, "Failed to extract a valid fullscreen mode.");
		mode = sf::VideoMode(dimensions.preset);
		is_fullscreen = false;
	}
	is_fullscreen ? window.create(mode, title, sf::State::Fullscreen) : window.create(mode, title, sf::Style::Default & ~sf::Style::Resize);
	dimensions.current = is_fullscreen ? dimensions.display : dimensions.preset;
}

void WindowManager::restore_view() { window.setView(game_view); }

void WindowManager::set_screencap() {
	if (!screencap.resize(window.getSize())) { NANI_LOG_WARN(m_logger, "Failed to resize screencap."); }
}

void WindowManager::resize() {
	dimensions.current = window.getSize();
	game_view = sf::View(sf::FloatRect({}, {static_cast<float>(dimensions.current.x), static_cast<float>(dimensions.current.y)}));
	window.setView(game_view);
}

} // namespace pi
