
#include "fornani/automa/GameState.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

GameState::GameState(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number) : player(&player), hud(svc, player) {}

void GameState::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	svc.soundboard.play_sounds(engine, svc);
	if (m_console) {
		m_console.value()->update(svc);
		if (m_console.value()->exit_requested()) { m_console = {}; }
	}
}

} // namespace fornani::automa
