
#include <imgui.h>
#include <editor/automa/Editor.hpp>
#include <editor/automa/Metagrid.hpp>
#include <editor/setup/EditorApplication.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <cassert>
#include <imgui-SFML.h>

namespace pi {

EditorApplication::EditorApplication(char** argv) : metadata(game_info, m_services.finder), m_services(argv, metadata, window, *m_engine) {

	// load app resources
	game_info = *dj::Json::from_file((m_services.finder.paths.editor / "data/config/version.json").string().c_str());
	assert(!game_info.is_null());

	NANI_LOG_INFO(m_logger, "> Launching {}", metadata.long_title());

	app_settings = *dj::Json::from_file((m_services.finder.paths.editor / "data/config/settings.json").string().c_str());
	assert(!app_settings.is_null());

	// create window
	window.create(metadata.long_title(), app_settings["fullscreen"].as_bool(), {1920, 1080});
	window.set();

	// set app icon
	sf::Image icon{};
	if (!icon.loadFromFile((m_services.finder.paths.editor / "app" / "icon.png").string())) { NANI_LOG_WARN(m_logger, "Failed to load Icon."); }
	window.get().setIcon({32, 32}, icon.getPixelsPtr());

	if (!ImGui::SFML::Init(window.get())) { NANI_LOG_ERROR(m_logger, "Failed to init SFML window."); };
	window.get().clear();
	window.get().display();

	user_data = *dj::Json::from_file((m_services.finder.paths.editor / "data" / "config" / "user.json").string().c_str());
	assert(!user_data.is_null());
	m_services.finder.paths.region = user_data["region"] ? user_data["region"].as_string() : "config";
	m_services.finder.paths.room_name = user_data["room"] ? user_data["room"].as_string() : "new_file.json";
	m_state = std::make_unique<Editor>(m_services);
	m_current_state = EditorStateType::editor;
}

void EditorApplication::run(char** argv) {
	while (window.get().isOpen()) {
		while (std::optional const event = window.get().pollEvent()) {
			ImGui::SFML::ProcessEvent(window.get(), *event);
			if (event->is<sf::Event::Closed>()) {
				shutdown();
				return;
			}
			if (auto const* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
				if (key_pressed->scancode == sf::Keyboard::Scancode::F12) { continue; }
			}
			m_state->handle_events(*event, window.get());
		}
		auto to_state = m_state->run(argv);
		if (to_state != m_current_state) {
			switch (to_state) {
			case EditorStateType::editor: m_state = std::make_unique<Editor>(m_services); break;
			case EditorStateType::metagrid: m_state = std::make_unique<Metagrid>(m_services); break;
			}
			m_current_state = to_state;
		}
	}
}

void EditorApplication::shutdown() {
	user_data["region"] = m_services.finder.paths.region;
	user_data["room"] = m_services.finder.paths.room_name;
	if (!user_data.to_file((m_services.finder.paths.editor / "data" / "config" / "user.json").string().c_str())) { NANI_LOG_WARN(m_logger, "Failed to log user data."); }
}

} // namespace pi
