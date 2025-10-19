
#pragma once

#include "fornani/automa/MenuState.hpp"

namespace fornani::automa {

class StatSheet final : public MenuState {
  public:
	StatSheet(ServiceProvider& svc, player::Player& player);
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
