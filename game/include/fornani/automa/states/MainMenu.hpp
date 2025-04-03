
#pragma once

#include "fornani/automa/GameState.hpp"

namespace fornani::automa {

class MainMenu final : public GameState {
  public:
	MainMenu(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	void tick_update(ServiceProvider& svc) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, WindowManager& win) override;

	sf::Sprite title;
	sf::Text subtitle;
	sf::Text instruction;
	util::Cooldown loading{16};
};

} // namespace fornani::automa
