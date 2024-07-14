
#pragma once

#include "../GameState.hpp"

namespace automa {

class CreditsMenu : public automa::GameState {
  public:
	CreditsMenu() = default;
	CreditsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id = 0);
	void init(ServiceProvider& svc, int room_number);
	void handle_events(ServiceProvider& svc, sf::Event& event);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);

	
};

} // namespace automa
