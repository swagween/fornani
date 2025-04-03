
#pragma once

#include "fornani/automa/GameState.hpp"

namespace fornani::automa {

class OptionsMenu final : public GameState {
  public:
	OptionsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	void tick_update(ServiceProvider& svc) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, WindowManager& win) override;
};

} // namespace fornani::automa
