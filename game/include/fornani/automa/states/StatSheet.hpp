
#pragma once

#include "fornani/automa/GameState.hpp"

namespace automa {

class StatSheet : public automa::GameState {
  public:
	StatSheet(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);

  private:
	sf::Text title;
	sf::Text stats;
	sf::Font stat_font{};
	util::Cooldown loading{2};
};

} // namespace automa
