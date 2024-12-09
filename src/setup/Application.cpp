#include "Application.hpp"

namespace fornani {

Application::Application(char** argv) {

	finder.setResourcePath(argv);
	std::cout << "Resource path: " << finder.resource_path << std::endl;

	// load app resources
	game_info = dj::Json::from_file((finder.resource_path + "/data/config/version.json").c_str());
	assert(!game_info.is_null());

	t_loading.loadFromFile(finder.resource_path + "/image/gui/loading.png");
	loading.setTexture(t_loading);
	metadata.title = game_info["title"].as_string();
	metadata.build = game_info["build"].as_string();
	metadata.major = game_info["version"]["major"].as<int>();
	metadata.minor = game_info["version"]["minor"].as<int>();
	metadata.hotfix = game_info["version"]["hotfix"].as<int>();
	std::cout << "> Launching " << metadata.long_title() << "\n";

	app_settings = dj::Json::from_file((finder.resource_path + "/data/config/settings.json").c_str());
	assert(!app_settings.is_null());

	// create window
	window.create(metadata.long_title(), static_cast<bool>(app_settings["fullscreen"].as_bool()));
	window.set();

	// set app icon
	sf::Image icon{};
	icon.loadFromFile(finder.resource_path + "/image/app/icon.png");
	window.get().setIcon(256, 256, icon.getPixelsPtr());

	ImGui::SFML::Init(window.get());
	window.get().clear();
	window.get().draw(loading);
	window.get().display();
}

void Application::launch(char** argv, bool demo, int room_id, std::filesystem::path levelpath, sf::Vector2<float> player_position) {
	std::unique_ptr game = std::make_unique<Game>(argv, window, metadata);
	game->run(demo, room_id, levelpath, player_position);
}

} // namespace fornani
