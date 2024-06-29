
#include "Intro.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

Intro::Intro(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id), map(svc, player, console) {
	title.setPosition(0, 0);
	title.setSize(static_cast<sf::Vector2f>(svc.constants.screen_dimensions));
	title.setFillColor(svc.styles.colors.ui_black);
	console = gui::Console(svc);
	map.load(svc, "/level/FIRSTWIND_CLOSET_03");
	svc.music.load("respite");
	svc.music.play_looped(20);
	console.set_source(svc.text.basic);
	float ppx = svc.data.get_save()["player_data"]["position"]["x"].as<float>();
	float ppy = svc.data.get_save()["player_data"]["position"]["y"].as<float>();
	sf::Vector2f player_pos = {ppx, ppy};
	player.set_position(player_pos);
}

void Intro::init(ServiceProvider& svc, std::string_view room) {}

void Intro::handle_events(ServiceProvider& svc, sf::Event& event) {
	svc.controller_map.handle_mouse_events(event);
	svc.controller_map.handle_joystick_events(event);
	if (event.type == sf::Event::EventType::KeyPressed) { svc.controller_map.handle_press(event.key.code); }
	if (event.type == sf::Event::EventType::KeyReleased) { svc.controller_map.handle_release(event.key.code); }
	if (svc.controller_map.label_to_control.at("menu_toggle_secondary").triggered()) { toggle_pause_menu(svc); }
}

void Intro::tick_update(ServiceProvider& svc) {
	player->update(console, inventory_window);
	player->controller.prevent_movement();
	map.update(svc, console, inventory_window);

	console.load_and_launch("intro");
	if (console.is_complete()) {
		svc.state_controller.actions.set(automa::Actions::intro_done);
		svc.state_controller.actions.set(automa::Actions::trigger);
	}
	map.debug_mode = debug_mode;

	svc.controller_map.reset_triggers();
	player->controller.clean();
	svc.soundboard.play_sounds(svc);
	player->flags.triggers = {};

	map.background->update(svc, {});
	console.end_tick();
}

void Intro::frame_update(ServiceProvider& svc) {
	pause_window.update(svc, *player);
	hud.update(svc, *player);
	svc.controller_map.reset_triggers();
	pause_window.clean_off_trigger();
}

void Intro::render(ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(title);
	pause_window.render(svc, *player, win);
	//map.render_background(svc, win, {});
	//map.render(svc, win, {});
	map.render_console(svc, console, win);

	map.transition.render(win);
}

void Intro::toggle_pause_menu(ServiceProvider& svc) {
	pause_window.active() ? pause_window.close() : pause_window.open();
	svc.ticker.paused() ? svc.ticker.unpause() : svc.ticker.pause();
	svc.controller_map.reset_triggers();
}

} // namespace automa
