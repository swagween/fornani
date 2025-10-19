
#pragma once

#include "fornani/automa/MenuState.hpp"

namespace fornani::automa {

class OptionsMenu final : public MenuState {
  public:
	OptionsMenu(ServiceProvider& svc, player::Player& player);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
};

} // namespace fornani::automa
