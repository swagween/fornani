#include "WindowManager.hpp"
#include <iostream>

namespace fornani {
	
void WindowManager::set() {
	game_view = sf::View(sf::FloatRect({}, {static_cast<float>(screen_dimensions.x), static_cast<float>(screen_dimensions.y)}));
	// set view and veiwport for fullscreen mode
	auto aspect_ratio = static_cast<float>(aspects.x) / static_cast<float>(aspects.y);
	auto display_ratio = static_cast<float>(display_dimensions.x) / static_cast<float>(display_dimensions.y);
	auto letterbox = std::min(display_ratio, aspect_ratio) / std::max(display_ratio, aspect_ratio);
	auto vertical = display_ratio < aspect_ratio;
	auto resize_ratio = vertical ? sf::Vector2<float>(1.f, letterbox) : sf::Vector2<float>(letterbox, 1.f);
	auto offset = vertical ? sf::Vector2<float>(0.f, (1.f - letterbox) * 0.5f) : sf::Vector2<float>((1.f - letterbox) * 0.5f, 0.f);
	game_port = is_fullscreen ? sf::FloatRect{offset.x, offset.y, resize_ratio.x, resize_ratio.y} : sf::FloatRect(0.f, 0.f, 1.f, 1.f);
	game_view.setViewport(game_port);
	window.setView(game_view);
	if (is_fullscreen) { window.setMouseCursorVisible(false); }

	screencap.create(window.getSize().x, window.getSize().y);
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);
}

void fornani::WindowManager::create(std::string title, bool const fullscreen) {
	is_fullscreen = fullscreen;
	// set window constants
	screen_dimensions = {aspects.x / 4, aspects.y / 4};
	display_dimensions = {static_cast<unsigned>(sf::VideoMode::getDesktopMode().width), static_cast<unsigned>(sf::VideoMode::getDesktopMode().height)};
	mode = fullscreen ? sf::VideoMode(display_dimensions.x, display_dimensions.y) : sf::VideoMode(screen_dimensions.x, screen_dimensions.y);
	if (!mode.isValid()) {
		std::cout << "Number of valid fullscreen modes: " << mode.getFullscreenModes().size() << "\n";
		std::cout << "Failed to extract a valid fullscreen mode.\n";
		mode = sf::VideoMode(screen_dimensions.x, screen_dimensions.y);
		is_fullscreen = false;
	}
	is_fullscreen ? window.create(mode, title, sf::Style::Fullscreen) : window.create(mode, title, sf::Style::Default);
}


void WindowManager::restore_view() { window.setView(game_view); }

void WindowManager::set_screencap() { screencap.create(window.getSize().x, window.getSize().y); }

} // namespace fornani