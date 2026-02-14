
#pragma once

#include <fornani/automa/MenuState.hpp>

namespace fornani::automa {

class ThemesMenu final : public MenuState {
  public:
	ThemesMenu(ServiceProvider& svc, player::Player& player);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
};

} // namespace fornani::automa
