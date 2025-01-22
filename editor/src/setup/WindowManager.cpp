#include "editor/setup/WindowManager.hpp"
#include <iostream>

namespace pi {
	
void WindowManager::set() {

	game_view = sf::View(sf::FloatRect({}, {static_cast<float>(dimensions.current.x), static_cast<float>(dimensions.current.y)}));

	// set view and veiwport for fullscreen mode
	game_port = sf::FloatRect(0.f, 0.f, 1.f, 1.f);
	game_view.setViewport(game_port);
	window.setView(game_view);

	screencap.create(window.getSize().x, window.getSize().y);
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);
}

void WindowManager::create(std::string title, bool const fullscreen) {
	is_fullscreen = fullscreen;
	// set window constants
	dimensions.display = {static_cast<unsigned>(sf::VideoMode::getDesktopMode().width), static_cast<unsigned>(sf::VideoMode::getDesktopMode().height)};
	mode = is_fullscreen ? sf::VideoMode(dimensions.display.x, dimensions.display.y) : sf::VideoMode(dimensions.preset.x, dimensions.preset.y);
	if (!mode.isValid()) {
		std::cout << "Number of valid fullscreen modes: " << mode.getFullscreenModes().size() << "\n";
		std::cout << "Failed to extract a valid fullscreen mode.\n";
		mode = sf::VideoMode(dimensions.preset.x, dimensions.preset.y);
		is_fullscreen = false;
	}
	is_fullscreen ? window.create(mode, title, sf::Style::Fullscreen) : window.create(mode, title, sf::Style::Default);
	dimensions.current = is_fullscreen ? dimensions.display : dimensions.preset;
}

void WindowManager::restore_view() { window.setView(game_view); }

void WindowManager::set_screencap() { screencap.create(window.getSize().x, window.getSize().y); }

void WindowManager::resize() {
	dimensions.current = window.getSize();
	game_view = sf::View(sf::FloatRect({}, {static_cast<float>(dimensions.current.x), static_cast<float>(dimensions.current.y)}));
	window.setView(game_view);
}

} // namespace pi
