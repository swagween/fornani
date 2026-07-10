
#pragma once

#include <djson/json.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/MenuTheme.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/systems/InputSystem.hpp>

namespace fornani {

class Localization;

class UserSettings {
  public:
	UserSettings(ResourceFinder& finder);
	void set_user_settings(automa::ServiceProvider& svc, Localization& localization);
	void set_user_controls(input::InputSystem& input);
	void save_user_settings(automa::ServiceProvider& svc);
	void save_user_controls(automa::ServiceProvider& svc);
	void reset_user_controls(automa::ServiceProvider& svc);
	void serialize_control_binding(std::string_view action, std::string_view binding, bool primary = true);
	void set_theme(std::string_view to);

	[[nodiscard]] auto get_theme() -> MenuTheme& { return m_theme; }
	[[nodiscard]] auto get_json() const& -> dj::Json { return m_settings; }

  private:
	void write_to_file(ResourceFinder& finder);

  private:
	dj::Json m_settings{};
	dj::Json m_controls{};
	dj::Json m_menu_themes{};
	MenuTheme m_theme{};
	ResourceFinder* m_finder;
	io::Logger m_logger{"UserSettings"};
};

} // namespace fornani
