
#pragma once

#include "../GameState.hpp"

namespace automa {

class ControlsMenu : public automa::GameState {
  public:
	ControlsMenu(ServiceProvider& svc, player::Player& player, std::string_view room_name = "", int room_number = 0);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);
	void refresh_controls(ServiceProvider& svc);
	void restore_defaults(ServiceProvider& svc);

  private:
	void change_scene(ServiceProvider& svc, std::string_view scene); // FIXME this is a hack to work with GameState; eliminate options from GameState

	float center_offset{320.f};
	std::vector<sf::Text> control_list{};
	util::Cooldown loading{};
	sf::Text instruction;
	bool option_is_selected{};
	bool binding_mode{};
	sf::RectangleShape debug{};
};

} // namespace automa
