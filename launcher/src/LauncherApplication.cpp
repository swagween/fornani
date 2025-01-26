
#include "launcher/LauncherApplication.hpp"

#include <tracy/Tracy.hpp>

namespace game
{
void LauncherApplication::init(char** argv) {
	ZoneScopedN("Application::Application");
	std::cout << "Resource path: " << m_finder.resource_path() << std::endl;
	std::cout << "> Launching " << m_metadata.long_title() << "\n";

	m_app_settings = dj::Json::from_file((m_finder.resource_path() + "/data/config/settings.json").c_str());
	assert(!m_app_settings.is_null());

	// create window
	m_window.create(m_metadata.long_title(), static_cast<bool>(m_app_settings["fullscreen"].as_bool()));
	m_window.set();

	// set app icon
	sf::Image icon{};
	if (!icon.loadFromFile(m_finder.resource_path() + "/image/app/icon.png")) { std::cout << "Failed to load application icon.\n"; };
	m_window.get().setIcon({256, 256}, icon.getPixelsPtr());

	m_window.get().clear();
	m_window.get().draw(m_loading);
	m_window.get().display();
}

void LauncherApplication::launch(char** argv, bool demo, int room_id, std::filesystem::path levelpath, sf::Vector2<float> player_position) {
	std::unique_ptr game = std::make_unique<fornani::Game>(argv, m_window, m_metadata);
	game->run(demo, room_id, levelpath, player_position);
}
}
