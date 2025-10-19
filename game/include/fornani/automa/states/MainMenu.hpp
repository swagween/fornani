
#pragma once

#include "fornani/automa/MenuState.hpp"

namespace fornani::automa {

class MainMenu final : public MenuState {
  public:
	MainMenu(ServiceProvider& svc, player::Player& player);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;

	sf::Sprite title;
	sf::Text subtitle;
	sf::Text instruction;
	util::Cooldown loading{16};
};

} // namespace fornani::automa
