
#pragma once

#include "fornani/automa/GameState.hpp"

namespace fornani::automa {

class CreditsMenu final : public GameState {
  public:
	CreditsMenu(ServiceProvider& svc, player::Player& player, std::string_view room_name = "", int room_number = 0);
	void tick_update(ServiceProvider& svc) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, WindowManager& win) override;
};

} // namespace fornani::automa
