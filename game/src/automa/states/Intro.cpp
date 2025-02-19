
#include "fornani/automa/states/Intro.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

Intro::Intro(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number) : GameState(svc, player, scene, room_number) {
	title.setSize(static_cast<sf::Vector2f>(svc.constants.screen_dimensions));
	title.setFillColor(svc.styles.colors.ui_black);
	console = gui::Console(svc);
	svc.music.load(svc.finder, "respite");
	svc.music.play_looped(20);
	console.set_source(svc.text.basic);
	float ppx = svc.data.get_save()["player_data"]["position"]["x"].as<float>();
	float ppy = svc.data.get_save()["player_data"]["position"]["y"].as<float>();
	sf::Vector2f player_pos = {ppx, ppy};
	player.set_position({300, 260});
}

void Intro::tick_update(ServiceProvider& svc) {
	/*if (pause_window.consume_exited()) { toggle_pause_menu(svc); }
	if (pause_window.active()) {
		svc.controller_map.set_action_set(config::ActionSet::Menu);
		if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_toggle_pause).triggered) { toggle_pause_menu(svc); }
		pause_window.update(svc, console, true);
		return;
	}*/
	svc.controller_map.set_action_set(config::ActionSet::Menu);
	if (svc.state_controller.actions.test(Actions::main_menu)) {
		svc.state_controller.actions.set(automa::Actions::trigger);
		return;
	}
	console.load_and_launch("intro");
	if (console.is_complete()) {
		svc.state_controller.actions.set(automa::Actions::intro_done);
		svc.state_controller.actions.set(automa::Actions::trigger);
		player->cooldowns.tutorial.start();
		player->tutorial.current_state = text::TutorialFlags::jump;
		player->tutorial.turn_on();
		player->tutorial.trigger();
		svc.ticker.in_game_seconds_passed = {};
	}

	player->controller.clean();
	svc.soundboard.play_sounds(svc);
	player->flags.triggers = {};

	// pause_window.update(svc, console, true);
}

void Intro::frame_update(ServiceProvider& svc) { hud.update(svc, *player); }

void Intro::render(ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(title);
	// pause_window.render(svc, *player, win);
}

void Intro::toggle_pause_menu(ServiceProvider& svc) {
	/*if (pause_window.active()) {
		pause_window.close();
		svc.soundboard.flags.console.set(audio::Console::done);
	} else {
		pause_window.open();
		svc.soundboard.flags.console.set(audio::Console::menu_open);
		svc.soundboard.play_sounds(svc);
	}*/
	svc.ticker.paused() ? svc.ticker.unpause() : svc.ticker.pause();
}

} // namespace fornani::automa
