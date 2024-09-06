
#pragma once

#include "../GameState.hpp"

namespace automa {

class ControlsMenu : public automa::GameState {
  public:
	ControlsMenu() = default;
	ControlsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id = 0);
	void init(ServiceProvider& svc, int room_number);
	void handle_events(ServiceProvider& svc, sf::Event& event);
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
	sf::Text instruction{};
	bool option_is_selected{};
	sf::RectangleShape debug{};
};

} // namespace automa
