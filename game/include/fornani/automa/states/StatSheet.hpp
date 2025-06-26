
#pragma once

#include "fornani/automa/GameState.hpp"

namespace fornani::automa {

class StatSheet final : public GameState {
  public:
	StatSheet(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;

  private:
	sf::Text title;
	sf::Text stats;
	sf::Font stat_font{};
	util::Cooldown loading{2};
};

} // namespace fornani::automa
