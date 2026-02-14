
#include "fornani/core/Application.hpp"
#include "fornani/graphics/Colors.hpp"

namespace fornani {

void Application::init(char** argv, std::pair<bool, bool> demo_fullscreen) {
	NANI_LOG_INFO(m_logger, "> Launching");
	NANI_LOG_INFO(m_logger, "Resource path: {}", m_finder.resource_path());

	m_app_settings = *dj::Json::from_file((m_finder.resource_path() + "/data/config/settings.json").c_str());
	assert(!m_app_settings.is_null());

	m_backdrop_color = sf::Color{m_app_settings["backdrop"][0].as<uint8_t>(), m_app_settings["backdrop"][1].as<uint8_t>(), m_app_settings["backdrop"][2].as<uint8_t>()};

	// create window
	auto fullscreen = demo_fullscreen.first ? demo_fullscreen.second : static_cast<bool>(m_app_settings["fullscreen"].as_bool());
	m_window.create(m_metadata.long_title(), fullscreen, {960, 512});
	m_window.set();

	auto entire_window = sf::View(sf::FloatRect{{}, sf::Vector2f{sf::VideoMode::getDesktopMode().size}});
	auto background = sf::RectangleShape{sf::Vector2f{sf::VideoMode::getDesktopMode().size}};
	background.setFillColor(m_backdrop_color);

	m_window.get().clear();
	if (m_window.is_fullscreen()) { m_window.get().setView(entire_window); }
	m_window.get().draw(background);
	m_window.restore_view();
	m_window.get().display();
}

void Application::launch(char** argv, bool demo, int room_id, std::filesystem::path levelpath, sf::Vector2f player_position) {
	std::unique_ptr game = std::make_unique<Game>(argv, m_window, m_metadata, *m_engine);
	game->set_file(m_file);
	game->run(*m_engine, demo, room_id, levelpath, player_position);
}

} // namespace fornani
