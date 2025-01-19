
#pragma once

#include "../GameState.hpp"

namespace automa {

class OptionsMenu : public automa::GameState {
  public:
	OptionsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);
	
};

} // namespace automa
