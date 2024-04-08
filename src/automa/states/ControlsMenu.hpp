
#pragma once

#include "../GameState.hpp"

namespace automa {

class ControlsMenu : public automa::GameState {
  public:
	ControlsMenu() = default;
	ControlsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id = 0);
	void init(ServiceProvider& svc, std::string_view room = "");
	void setTilesetTexture(ServiceProvider& svc, sf::Texture& t);
	void handle_events(ServiceProvider& svc, sf::Event& event);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);

  private:
	float center_offset{200.f};
	std::vector<sf::Text> control_list{};
};

} // namespace automa
