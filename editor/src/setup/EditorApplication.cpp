#include "editor/setup/EditorApplication.hpp"
#include "editor/automa/Editor.hpp"

#include <imgui.h>
#include <cassert>
#include <imgui-SFML.h>

namespace pi {

EditorApplication::EditorApplication(char** argv) : finder(argv), metadata(game_info, finder) {

	// load app resources
	game_info = *dj::Json::from_file((finder.paths.editor / "data/config/version.json").string().c_str());
	assert(!game_info.is_null());

	NANI_LOG_INFO(m_logger, "> Launching {}", metadata.long_title());

	app_settings = *dj::Json::from_file((finder.paths.editor / "data/config/settings.json").string().c_str());
	assert(!app_settings.is_null());

	// create window
	window.create(metadata.long_title(), app_settings["fullscreen"].as_bool(), {1920, 1080});
	window.set();

	// set app icon
	sf::Image icon{};
	if (!icon.loadFromFile((finder.paths.editor / "app" / "icon.png").string())) { NANI_LOG_WARN(m_logger, "Failed to load Icon."); }
	window.get().setIcon({32, 32}, icon.getPixelsPtr());

	if (!ImGui::SFML::Init(window.get())) { NANI_LOG_ERROR(m_logger, "Failed to init SFML window."); };
	window.get().clear();
	window.get().display();
}

void EditorApplication::launch(char** argv) {
	std::unique_ptr editor = std::make_unique<Editor>(argv, window, finder, metadata, *m_engine);
	editor->run();
}

} // namespace pi
