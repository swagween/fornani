#include "fornani/setup/Application.hpp"

#include <tracy/Tracy.hpp>

namespace fornani {

void Application::init(char** argv) {
	ZoneScopedN("Application::Application");
	std::cout << "Resource path: " << finder.resource_path() << std::endl;
	std::cout << "> Launching " << metadata.long_title() << "\n";

	app_settings = dj::Json::from_file((finder.resource_path() + "/data/config/settings.json").c_str());
	assert(!app_settings.is_null());

	// create window
	window.create(metadata.long_title(), static_cast<bool>(app_settings["fullscreen"].as_bool()));
	window.set();

	// set app icon
	sf::Image icon{};
	if (!icon.loadFromFile(finder.resource_path() + "/image/app/icon.png")) { std::cout << "Failed to load application icon.\n"; };
	window.get().setIcon({256, 256}, icon.getPixelsPtr());

	window.get().clear();
	window.get().draw(loading);
	window.get().display();
}

void Application::launch(char** argv, bool demo, int room_id, std::filesystem::path levelpath, sf::Vector2<float> player_position) {
	std::unique_ptr game = std::make_unique<Game>(argv, window, metadata);
	game->run(demo, room_id, levelpath, player_position);
	std::cout << "> Exited\n";
}

} // namespace fornani
