
#pragma once

#include "fornani/automa/GameState.hpp"

namespace fornani::automa {

class MainMenu : public GameState {
  public:
	MainMenu(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);

	sf::Sprite title;
	sf::Text subtitle;
	sf::Text instruction;
	util::Cooldown loading{16};
};

} // namespace automa
