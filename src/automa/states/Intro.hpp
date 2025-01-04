
#pragma once

#include "../GameState.hpp"
#include "../../level/Camera.hpp"

namespace automa {

class Intro : public GameState {
  public:
	Intro() = default;
	Intro(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int id = 0);
	void init(ServiceProvider& svc, int room_number);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);
	void toggle_pause_menu(ServiceProvider& svc);

	world::Map map;
	sf::RectangleShape title{};
};

} // namespace automa
