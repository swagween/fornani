
#include "fornani/automa/GameState.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

GameState::GameState(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number) : player(&player), hud(svc, player), p_context{svc} {}

void GameState::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	svc.notifications.update(svc);
	if (p_context.console) {
		p_context.console.value()->update(svc);
		if (p_context.console.value()->exit_requested()) { p_context.console = {}; }
	}
}

} // namespace fornani::automa
