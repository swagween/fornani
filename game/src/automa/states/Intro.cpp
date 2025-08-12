
#include "fornani/automa/states/Intro.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

Intro::Intro(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number) : GameState(svc, player, scene, room_number) {
	title.setSize(static_cast<sf::Vector2f>(svc.window->i_screen_dimensions()));
	title.setFillColor(colors::ui_black);
	svc.music_player.load(svc.finder, "respite");
	svc.music_player.play_looped();
	float ppx = svc.data.get_save()["player_data"]["position"]["x"].as<float>();
	float ppy = svc.data.get_save()["player_data"]["position"]["y"].as<float>();
	sf::Vector2f player_pos = {ppx, ppy};
	player.set_position({300, 260});
	m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "intro", gui::OutputType::gradual);
}

void Intro::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	GameState::tick_update(svc, engine);
	svc.controller_map.set_action_set(config::ActionSet::Menu);
	if (svc.state_controller.actions.test(Actions::main_menu)) {
		svc.state_controller.actions.set(automa::Actions::trigger);
		return;
	}
	if (!m_console) {
		svc.state_controller.actions.set(automa::Actions::intro_done);
		svc.state_controller.actions.set(automa::Actions::trigger);
		player->cooldowns.tutorial.start();
		svc.ticker.in_game_seconds_passed = {};
	}

	player->controller.clean();
	player->flags.triggers = {};
}

void Intro::frame_update(ServiceProvider& svc) { hud.update(svc, *player); }

void Intro::render(ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(title);
	if (m_console) {
		m_console.value()->render(win);
		m_console.value()->write(win, false);
	}
}

void Intro::toggle_pause_menu(ServiceProvider& svc) { svc.ticker.paused() ? svc.ticker.unpause() : svc.ticker.pause(); }

} // namespace fornani::automa
