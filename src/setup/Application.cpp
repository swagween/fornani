#include "Application.hpp"

namespace fornani {

Application::Application(char** argv) {

	finder.setResourcePath(argv);

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
	settings.fullscreen = static_cast<bool>(app_settings["fullscreen"].as_bool());

	//window constants
	sf::Vector2<int> aspects{3840, 2048};
	window_utilities.screen_dimensions = {aspects.x / 4, aspects.y / 4};
	sf::Vector2u display_dimensions{static_cast<unsigned>(sf::VideoMode::getDesktopMode().width), static_cast<unsigned>(sf::VideoMode::getDesktopMode().height)};

	//create window
	window_utilities.mode = settings.fullscreen ? sf::VideoMode(display_dimensions.x, display_dimensions.y) : sf::VideoMode(window_utilities.screen_dimensions.x, window_utilities.screen_dimensions.y);
	if (!window_utilities.mode.isValid()) {
		std::cout << "Number of valid fullscreen modes: " << window_utilities.mode.getFullscreenModes().size() << "\n";
		std::cout << "Failed to extract a valid fullscreen mode.\n";
		window_utilities.mode = sf::VideoMode(window_utilities.screen_dimensions.x, window_utilities.screen_dimensions.y);
		settings.fullscreen = false;
	}
	settings.fullscreen ? window.create(window_utilities.mode, metadata.long_title(), sf::Style::Fullscreen) : window.create(window_utilities.mode, metadata.long_title(), sf::Style::Default);
	window_utilities.game_view = sf::View(sf::FloatRect({}, {static_cast<float>(window_utilities.screen_dimensions.x), static_cast<float>(window_utilities.screen_dimensions.y)}));

	// set view and veiwport for fullscreen mode
	auto aspect_ratio = static_cast<float>(aspects.x) / static_cast<float>(aspects.y);
	auto display_ratio = static_cast<float>(display_dimensions.x) / static_cast<float>(display_dimensions.y);
	std::cout << "aspect ratio: " << aspect_ratio << "\n";
	std::cout << "display ratio: " << display_ratio << "\n";
	auto letterbox = std::min(display_ratio, aspect_ratio) / std::max(display_ratio, aspect_ratio);
	auto vertical = display_ratio < aspect_ratio;
	auto resize_ratio = vertical ? sf::Vector2<float>(1.f, letterbox) : sf::Vector2<float>(letterbox, 1.f);
	auto offset = vertical ? sf::Vector2<float>(0.f, (1.f - letterbox) * 0.5f) : sf::Vector2<float>((1.f - letterbox) * 0.5f, 0.f);

	window_utilities.game_port = sf::FloatRect{offset.x, offset.y, resize_ratio.x, resize_ratio.y};
	window_utilities.game_view.setViewport(window_utilities.game_port);
	window.setView(window_utilities.game_view);
	
	// set app icon
	sf::Image icon{};
	icon.loadFromFile(finder.resource_path + "/image/app/icon.png");
	window.setIcon(256, 256, icon.getPixelsPtr());

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
