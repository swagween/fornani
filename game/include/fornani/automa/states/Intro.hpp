
#pragma once

#include "fornani/automa/GameState.hpp"
#include "fornani/level/Camera.hpp"

namespace fornani::automa {

class Intro : public GameState {
  public:
	Intro(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);
	void toggle_pause_menu(ServiceProvider& svc);

	world::Map map;
	sf::RectangleShape title{};
};

} // namespace automa
