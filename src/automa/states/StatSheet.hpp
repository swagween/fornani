
#pragma once

#include "../GameState.hpp"

namespace automa {

class StatSheet : public automa::GameState {
  public:
	StatSheet() = default;
	StatSheet(ServiceProvider& svc, player::Player& player, std::string_view scene, int id = 0);
	void init(ServiceProvider& svc, int room_number);
	void handle_events(ServiceProvider& svc, sf::Event& event);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);

  private:
	sf::Text title{};
	sf::Text stats{};
	sf::Font stat_font{};
	util::Cooldown loading{2};
};

} // namespace automa
