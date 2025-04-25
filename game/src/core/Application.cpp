
#include "fornani/core/Application.hpp"
#include "fornani/graphics/Colors.hpp"

#include "fornani/utils/Tracy.hpp"

namespace fornani {

void Application::init(char** argv, std::pair<bool, bool> demo_fullscreen) {
	NANI_ZoneScopedN("Application::Application");
	NANI_LOG_INFO(m_logger, "> Launching");
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
	if (m_window.fullscreen()) { m_window.get().setView(entire_window); }
	m_window.get().draw(background);
	m_window.restore_view();
	m_window.get().display();
}

void Application::launch(char** argv, bool demo, int room_id, std::filesystem::path levelpath, sf::Vector2<float> player_position) {
	std::unique_ptr game = std::make_unique<Game>(argv, m_window, m_metadata);
	game->run(demo, room_id, levelpath, player_position);
}

} // namespace fornani
