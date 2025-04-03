
#include "fornani/setup/WindowManager.hpp"

#include "app/app_icon_data.hpp" // embedded icon data

#include <imgui-SFML.h>

namespace fornani {

constexpr sf::Vector2i aspects{3840, 2048};
constexpr int factor{4};

void WindowManager::set() {
	m_game_view = sf::View(sf::FloatRect({}, {static_cast<float>(metadata.i_screen_dimensions().x), static_cast<float>(metadata.i_screen_dimensions().y)}));
	// set view and viewport for fullscreen mode
	auto const aspect_ratio = static_cast<float>(aspects.x) / static_cast<float>(aspects.y);
	auto const display_ratio = static_cast<float>(metadata.display_dimensions.x) / static_cast<float>(metadata.display_dimensions.y);
	auto const letterbox = std::min(display_ratio, aspect_ratio) / std::max(display_ratio, aspect_ratio);
	auto const vertical = display_ratio < aspect_ratio;
	auto resize_ratio = vertical ? sf::Vector2<float>(1.f, letterbox) : sf::Vector2<float>(letterbox, 1.f);
	auto offset = vertical ? sf::Vector2<float>(0.f, (1.f - letterbox) * 0.5f) : sf::Vector2<float>((1.f - letterbox) * 0.5f, 0.f);
	m_game_port = metadata.is_fullscreen() ? sf::FloatRect{{offset.x, offset.y}, {resize_ratio.x, resize_ratio.y}} : sf::FloatRect({}, {1.f, 1.f});
	m_window.setView(m_game_view);
	if (metadata.is_fullscreen()) { m_window.setMouseCursorVisible(false); }

	if (!screencap.resize(m_window.getSize())) { NANI_LOG_WARN(m_logger, "Failed to resize m_window."); }
	m_window.setVerticalSyncEnabled(true);
	m_window.setFramerateLimit(60);
	m_window.setKeyRepeatEnabled(false);
}

void WindowManager::create(std::string const& title, bool const fullscreen) {
	metadata.set_fullscreen(fullscreen);
	// set window constants
	metadata.set_screen_dimensions(aspects / factor);
	metadata.display_dimensions = {static_cast<unsigned>(sf::VideoMode::getDesktopMode().size.x), static_cast<unsigned>(sf::VideoMode::getDesktopMode().size.y)};
	m_mode = fullscreen ? sf::VideoMode(metadata.display_dimensions) : sf::VideoMode(metadata.u_screen_dimensions());
	if (!m_mode.isValid() && fullscreen) {
		NANI_LOG_WARN(m_logger,
					  "Number of valid fullscreen modes: {}\n"
					  "Failed to extract a valid fullscreen mode.",
					  sf::VideoMode::getFullscreenModes().size());
		m_mode = sf::VideoMode(metadata.u_screen_dimensions());
		metadata.set_fullscreen(false);
	}
	// set app icon
	sf::Image icon;
	if (!icon.loadFromMemory(generated::icon_png.data(), generated::icon_png.size())) { NANI_LOG_WARN(m_logger, "Failed to load application icon.\n"); };

	m_window = sf::RenderWindow(m_mode, title, metadata.is_fullscreen() ? sf::Style::Default : sf::Style::Default & ~sf::Style::Resize, metadata.is_fullscreen() ? sf::State::Fullscreen : sf::State::Windowed);

	m_window.setIcon(icon.getSize(), icon.getPixelsPtr());
}

void WindowManager::restore_view() { m_window.setView(m_game_view); }

void WindowManager::set_screencap() {
	if (!screencap.resize(m_window.getSize())) { NANI_LOG_WARN(m_logger, "Window resize failed!"); }
}

void WindowManager::take_screenshot(data::ResourceFinder& finder) {
	screencap.update(m_window);
	std::time_t const now = std::time(nullptr);
	std::time_t const time = std::time({});
	char time_string[std::size("yyyy-mm-ddThh:mm:ssZ")];
	std::strftime(std::data(time_string), std::size(time_string), "%FT%TZ", std::gmtime(&time));
	auto time_str = std::string{time_string};

	std::erase_if(time_str, [](auto const& c) { return c == ':' || isspace(c); });
	auto destination = std::filesystem::path{finder.paths.screenshots.string()};
	auto filename = std::filesystem::path{"screenshot_" + time_str + ".png"};
	auto target = destination / filename;
	if (screencap.copyToImage().saveToFile(target.string())) { NANI_LOG_INFO(m_logger, "screenshot {} saved to {}", filename.string(), destination.string()); }
}

} // namespace fornani
