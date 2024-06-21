
#pragma once

#include "../GameState.hpp"

namespace automa {

enum class Toggles { keyboard, gamepad };

class SettingsMenu : public automa::GameState {
  public:
	SettingsMenu() = default;
	SettingsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id = 0);
	void init(ServiceProvider& svc, std::string_view room = "");
	void handle_events(ServiceProvider& svc, sf::Event& event);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);

private:

	util::BitFlags<Toggles> toggles{};
	struct {
		sf::Text enabled{};
		sf::Text disabled{};
	} toggle_options{};
	struct {
		sf::Text music_volume{};
	} sliders{};
};

} // namespace automa
