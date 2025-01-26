
#pragma once

#include "fornani/automa/GameState.hpp"

namespace automa {

enum class Toggles { autosprint, tutorial, gamepad, music, fullscreen };
enum class MenuMode { adjust };

class SettingsMenu : public automa::GameState {
  public:
	SettingsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);
	[[nodiscard]] auto adjust_mode() const -> bool { return mode_flags.test(MenuMode::adjust); }

  private:
	util::BitFlags<Toggles> toggles{};
	util::BitFlags<MenuMode> mode_flags{};
	struct {
		sf::Text enabled;
		sf::Text disabled;
	} toggle_options;

	struct {
		sf::Text autosprint;
		sf::Text tutorial;
		sf::Text gamepad;
		sf::Text fullscreen;
	} toggleables;

	struct {
		sf::Text music_volume;
	} sliders;

	sf::Text music_label;
};

} // namespace automa
