
#include "fornani/core/Application.hpp"
#include "fornani/graphics/Colors.hpp"

#include "fornani/utils/Tracy.hpp"

namespace fornani {

void Application::init(char** argv, std::pair<bool, bool> demo_fullscreen) {
	NANI_ZoneScopedN("Application::Application");
	NANI_LOG_INFO(m_logger, "Launching {}", m_metadata.long_title());
	NANI_LOG_INFO(m_logger, "Resource path: {}", m_finder.resource_path());

	m_app_settings = dj::Json::from_file((m_finder.resource_path() + "/data/config/settings.json").c_str());
	assert(!m_app_settings.is_null());

	// create window
	auto fullscreen = demo_fullscreen.first ? demo_fullscreen.second : static_cast<bool>(m_app_settings["fullscreen"].as_bool());
	m_window.create(m_metadata.long_title(), fullscreen);
	m_window.set();

	auto entire_window = sf::View(sf::FloatRect{{}, sf::Vector2f{sf::VideoMode::getDesktopMode().size}});
	auto background = sf::RectangleShape{sf::Vector2f{sf::VideoMode::getDesktopMode().size}};
	background.setFillColor(colors::ui_black);

	m_window.get().clear();
	if (m_window.is_fullscreen()) { m_window.get().setView(entire_window); }
	m_window.get().draw(background);
	m_window.restore_view();
	char const* loading_screen = "/image/gui/loading.png";
	auto t_loading = sf::Texture{};
	if (!t_loading.loadFromFile(m_finder.resource_path() + loading_screen)) { NANI_LOG_WARN(m_logger, "Failed to load loading texture"); }
	auto loading = sf::Sprite{t_loading};
	m_window.get().draw(loading);
	m_window.get().display();
}

void Application::launch(char** argv, bool demo, int room_id, std::filesystem::path levelpath, sf::Vector2<float> player_position) {
	std::unique_ptr game = std::make_unique<Game>(argv, m_window, m_metadata);
	game->run(m_window, demo, room_id, levelpath, player_position);
	m_window.get().close();
}

} // namespace fornani
