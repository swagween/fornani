
#pragma once

#include "../GameState.hpp"

namespace automa {

class CreditsMenu : public automa::GameState {
  public:
	CreditsMenu(ServiceProvider& svc, player::Player& player, std::string_view room_name = "", int room_number = 0);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);

	
};

} // namespace automa
