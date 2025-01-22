#include "editor/setup/EditorApplication.hpp"
#include "editor/automa/Editor.hpp"

#include <cassert>
#include <imgui.h>
#include <imgui-SFML.h>

namespace pi {

EditorApplication::EditorApplication(char** argv) {

	finder.paths.local = finder.find_directory(argv[0], "assets");
	// load app resources
	game_info = dj::Json::from_file((finder.paths.local / "data/config/version.json").string().c_str());
	assert(!game_info.is_null());

	metadata.title = game_info["title"].as_string();
	metadata.build = game_info["build"].as_string();
	metadata.major = game_info["version"]["major"].as<int>();
	metadata.minor = game_info["version"]["minor"].as<int>();
	metadata.hotfix = game_info["version"]["hotfix"].as<int>();
	std::cout << "> Launching " << metadata.long_title() << "\n";

	app_settings = dj::Json::from_file((finder.paths.local / "data/config/settings.json").string().c_str());
	assert(!app_settings.is_null());

	// create window
	window.create(metadata.long_title(), static_cast<bool>(app_settings["fullscreen"].as_bool()));
	window.set();

	// set app icon
	sf::Image icon{};
	icon.loadFromFile((finder.paths.local / "app" / "icon.png").string());
	window.get().setIcon(32, 32, icon.getPixelsPtr());

	ImGui::SFML::Init(window.get());
	window.get().clear();
	window.get().display();
}

void EditorApplication::launch(char** argv) {
	std::unique_ptr editor = std::make_unique<Editor>(argv, window, finder);
	editor->run();
}

} // namespace pi
