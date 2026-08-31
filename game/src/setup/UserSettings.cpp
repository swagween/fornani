
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/setup/UserSettings.hpp>
#include <fornani/setup/localization/Localization.hpp>

namespace fornani {

UserSettings::UserSettings(ResourceFinder& finder) : m_finder{&finder} {
	auto settings_file = finder.paths.config / fs::path{"settings.json"};
	auto settings_template = finder.resource_path() / fs::path{"data/config/settings.json"};
	auto controls_file = finder.paths.config / fs::path{"controls.json"};
	auto controls_template = finder.resource_path() / fs::path{"data/config/controls.json"};
	finder.ensure_file_exists(settings_file, settings_template);
	finder.ensure_file_exists(controls_file, controls_template);

	NANI_LOG_INFO(m_logger, "Loading user settings from {}", settings_file.string());

	// general settings
	auto settings_result = dj::Json::from_file(settings_file.string());
	if (!settings_result) {
		NANI_LOG_ERROR(m_logger, "Failed to load user settings from {}.", settings_file.string());
		return;
	}
	m_settings = std::move(*settings_result);
	assert(!m_settings.is_null());

	// load theme
	auto themes_result = dj::Json::from_file((finder.resource_path() + "/data/gui/menu_themes.json").c_str());
	if (!themes_result) {
		NANI_LOG_ERROR(m_logger, "Failed to load themes!");
		return;
	}
	m_menu_themes = std::move(*themes_result);
	assert(!m_menu_themes.is_null());

	auto themestr = m_settings["theme"] ? m_settings["theme"].as_string() : "classic";
	m_theme = MenuTheme{m_menu_themes[themestr]};
	NANI_LOG_INFO(m_logger, "Loaded theme {}", themestr);

	// control bindings
	auto controls_result = dj::Json::from_file((finder.paths.config / "controls.json").string());
	if (!controls_result) { return; }

	m_controls = std::move(*controls_result);
	assert(!m_controls.is_null());
	assert(m_controls["controls"] && m_controls["controls"].is_object());
}

void UserSettings::set_user_settings(automa::ServiceProvider& svc, Localization& localization) {
	NANI_LOG_INFO(m_logger, "Setting user settings...");
	svc.input_system.set_setting(input::InputSystemSettings::auto_sprint, m_settings["auto_sprint"].as_bool());
	svc.set_tutorial(m_settings["tutorial"].as_bool());
	svc.input_system.set_setting(input::InputSystemSettings::gamepad_input_enabled, m_settings["gamepad"].as_bool());
	svc.music_player.volume.set_base(m_settings["music_volume"].as<double>());
	svc.ambience_player.volume.set_base(m_settings["ambience_volume"].as<double>());
	svc.soundboard.volume.set_base(m_settings["sfx_volume"].as<double>());
	svc.world_clock.set_military(m_settings["military_time"].as_bool());

	localization.set_language(m_settings["language"].as_string());
	NANI_LOG_INFO(m_logger, "Set Language to {}", localization.get_language_title());
}

void UserSettings::set_user_controls(input::InputSystem& input) {
	for (auto const& [key, item] : m_controls["controls"].as_object()) {
		assert(item.is_object());
		if (item.as_object().contains("primary_key")) { input.set_primary_keyboard_binding(input::action_from_string(key), input::scancode_from_string(item["primary_key"].as_string())); }
	}
	input.load_keyboard_controls(*m_finder);
}

void UserSettings::save_user_settings(automa::ServiceProvider& svc) {
	m_settings["auto_sprint"] = svc.input_system.is_autosprint_enabled();
	m_settings["tutorial"] = svc.tutorial();
	m_settings["gamepad"] = svc.input_system.is_gamepad_input_enabled();
	m_settings["music_volume"] = svc.music_player.volume.get_base();
	m_settings["ambience_volume"] = svc.ambience_player.volume.get_base();
	m_settings["sfx_volume"] = svc.soundboard.volume.get_base();
	m_settings["fullscreen"] = svc.fullscreen();
	m_settings["military_time"] = svc.world_clock.is_military();
	write_to_file(svc.finder);
}

void UserSettings::save_user_controls(automa::ServiceProvider& svc) {
	if (!m_controls.to_file((svc.finder.paths.config / "controls.json").string())) { NANI_LOG_ERROR(m_logger, "Failed to save controls layout!"); }
}

void UserSettings::reset_user_controls(automa::ServiceProvider& svc) {
	// control bindings
	auto controls_result = dj::Json::from_file((svc.finder.resource_path() + "/data/config/defaults.json").c_str());
	if (!controls_result) {
		NANI_LOG_ERROR(m_logger, "Failed to reset controls layout!");
		return;
	}

	m_controls = std::move(*controls_result);
}

void UserSettings::serialize_control_binding(std::string_view action, std::string_view binding, bool primary) {
	auto key = primary ? "primary_key" : "secondary_key";
	auto& destination = m_controls["controls"][action][key] = binding;
}

void UserSettings::set_theme(std::string_view to) {
	m_settings["theme"] = to;
	m_theme = MenuTheme{m_menu_themes[to]};
	write_to_file(*m_finder);
}

void UserSettings::set_language(std::string_view to) { m_settings["language"] = to; }

void UserSettings::write_to_file(ResourceFinder& finder) {
	auto save_path = (finder.paths.config / fs::path{"settings.json"}).string();
	if (!m_settings.to_file(save_path)) {
		NANI_LOG_ERROR(m_logger, "Failed to save user settings!");
	} else {
		NANI_LOG_INFO(m_logger, "Saved settings to {}", save_path);
	}
}

} // namespace fornani
