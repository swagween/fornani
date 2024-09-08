#include "Application.hpp"

namespace fornani {

Application::Application(char** argv) {

	finder.setResourcePath(argv);
	game_info = dj::Json::from_file((finder.resource_path + "/data/config/version.json").c_str());
	assert(!game_info.is_null());
	t_loading.loadFromFile(finder.resource_path + "/image/gui/loading.png");
	loading.setTexture(t_loading);
	metadata.title = game_info["title"].as_string();
	metadata.build = game_info["build"].as_string();
	metadata.major = game_info["version"]["major"].as<int>();
	metadata.minor = game_info["version"]["minor"].as<int>();
	metadata.hotfix = game_info["version"]["hotfix"].as<int>();
	std::cout << "> launching " << metadata.long_title() << "\n";

	//create window
	sf::Vector2<int> aspect_ratio{3840, 2160};
	sf::Vector2<int> screen_dimensions{aspect_ratio.x / 4, aspect_ratio.y / 4};
	window.create(sf::VideoMode(screen_dimensions.x, screen_dimensions.y), metadata.long_title());
	screencap.create(window.getSize().x, window.getSize().y);
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);

	ImGui::SFML::Init(window);
	window.clear();
	window.draw(loading);
	window.display();
}

void Application::launch(char** argv) {
	Game game{argv};
	game.run(window, screencap);
}

} // namespace fornani
