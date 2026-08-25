
#include <fornani/core/Application.hpp>

namespace fornani {

void Application::init(char** argv, DemoFlags demo_fullscreen) {
	NANI_LOG_INFO(m_logger, "Initializing {}", m_context.version.long_title());

	m_backdrop_color = m_context.settings.get_theme().backdrop;
	m_demo_flags = demo_fullscreen;
}

void Application::launch(char** argv, bool demo, int room_id, std::filesystem::path levelpath, sf::Vector2f player_position) {
	NANI_LOG_INFO(m_logger, "Launching {}", m_context.version.get_title());
	m_context.localization.set_language(m_context.settings.get_language_setting()); // default to english

	std::unique_ptr game = std::make_unique<Game>(argv, m_window, m_context, *m_engine);

	// create window
	auto fullscreen = m_demo_flags.is_demo ? m_demo_flags.value : static_cast<bool>(m_context.settings.get_json()["fullscreen"].as_bool());
	m_window.create(m_context.version.long_title(), fullscreen, {960, 512});
	m_window.set();

	// load game
	auto entire_window = sf::View(sf::FloatRect{{}, sf::Vector2f{sf::VideoMode::getDesktopMode().size}});
	auto background = sf::RectangleShape{sf::Vector2f{sf::VideoMode::getDesktopMode().size}};
	background.setFillColor(m_backdrop_color);
	m_window.get().clear();
	if (m_window.is_fullscreen()) { m_window.get().setView(entire_window); }
	m_window.get().draw(background);
	m_window.restore_view();
	m_window.get().display();

	// load settings
	m_context.settings.set_user_settings(game->get_services(), m_context.localization);
	m_context.settings.reset_user_controls(game->get_services());
	game->set_file(m_file);

	game->run(*m_engine, demo, room_id, levelpath, player_position);
}

} // namespace fornani
