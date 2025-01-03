
#pragma once

#include "../GameState.hpp"

namespace automa {

enum class Toggles { autosprint, tutorial, gamepad, music, fullscreen };
enum class MenuMode { adjust };

class SettingsMenu : public automa::GameState {
  public:
	SettingsMenu() = default;
	SettingsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id = 0);
	void init(ServiceProvider& svc, int room_number);
	void handle_events(ServiceProvider& svc, sf::Event& event);
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
