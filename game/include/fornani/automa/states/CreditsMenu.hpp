
#pragma once

#include "fornani/automa/GameState.hpp"

namespace fornani::automa {

class CreditsMenu : public GameState {
  public:
	CreditsMenu(ServiceProvider& svc, player::Player& player, std::string_view room_name = "", int room_number = 0);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);

	
};

} // namespace automa
