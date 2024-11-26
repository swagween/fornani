
#include "GameState.hpp"
#include "../service/ServiceProvider.hpp"

namespace automa {

GameState::GameState(ServiceProvider& svc, player::Player& player, std::string_view scene, int id)
	: player(&player), hud(svc, player, {20, 20}), inventory_window(svc), scene(scene), pause_window(svc) {
	font.loadFromFile(svc.text.title_font);
	font.setSmooth(false);
	subtitle_font.loadFromFile(svc.text.text_font);
	subtitle_font.setSmooth(false);

	top_buffer = svc.data.menu["config"][scene]["top_buffer"].as<float>();
}

} // namespace automa