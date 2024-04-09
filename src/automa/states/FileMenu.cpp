
#include "FileMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

FileMenu::FileMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {
	current_selection = 0;
	state = STATE::STATE_FILE;
	svc.data.load_blank_save(player);
	hud.set_corner_pad(svc, true); // display hud preview for each file in the center of the screen
	constrain_selection();
	svc.state_controller.next_state = svc.data.load_progress(player, current_selection);
	svc::cameraLocator.get().set_position({1, 1});
	player.set_position({(float)(cam::screen_dimensions.x / 2) + 80, 360});

	title.setPosition(0, 0);
	title.setSize(static_cast<sf::Vector2f>(cam::screen_dimensions));
	title.setFillColor(flcolor::ui_black);

	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);
}

void FileMenu::init(ServiceProvider& svc, std::string_view room) {}

void FileMenu::setTilesetTexture(ServiceProvider& svc, sf::Texture& t) {}

void FileMenu::handle_events(ServiceProvider& svc, sf::Event& event) {
	svc.controller_map.handle_mouse_events(event);
	svc.controller_map.handle_joystick_events(event);
	if (event.type == sf::Event::EventType::KeyPressed) { svc.controller_map.handle_press(event.key.code); }
	if (event.type == sf::Event::EventType::KeyReleased) { svc.controller_map.handle_release(event.key.code); }

	if (svc.controller_map.label_to_control.at("down").triggered()) {
		++current_selection;
		constrain_selection();
		svc.data.load_blank_save(*player);
		svc.data.load_progress(*player, current_selection);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.label_to_control.at("up").triggered()) {
		--current_selection;
		constrain_selection();
		svc.data.load_blank_save(*player);
		svc.data.load_progress(*player, current_selection);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.label_to_control.at("left").triggered()) {
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	if (svc.controller_map.label_to_control.at("main_action").triggered()) {
		constrain_selection();
		svc.data.load_progress(*player, current_selection, true);
		svc.state_controller.actions.set(Actions::trigger);
		svc.state_controller.actions.set(Actions::save_loaded);
		svc.soundboard.flags.menu.set(audio::Menu::select);
		svc.soundboard.flags.world.set(audio::World::load);
	}
	svc.controller_map.reset_triggers();
}

void FileMenu::tick_update(ServiceProvider& svc) {
	for (auto& option : options) { option.update(svc, current_selection); }
	constrain_selection();

	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection).left_offset);
	right_dot.set_target_position(options.at(current_selection).right_offset);

	hud.update(*player);
	for (auto& a : player->antennae) { a.collider.reset(); }

	player->collider.physics.acceleration = {};
	player->collider.physics.velocity = {};
	player->collider.physics.zero();
	player->flags.state.set(player::State::alive);
	player->collider.reset();
	player->controller.autonomous_walk();
	player->collider.flags.set(shape::State::grounded);

	player->update_weapon();
	player->update_animation();
	player->update_sprite();
	player->update_direction();
	player->apparent_position.x = player->collider.physics.position.x + player::PLAYER_WIDTH / 2;
	player->apparent_position.y = player->collider.physics.position.y;
	player->update_animation();
	player->update_antennae();

	svc.soundboard.play_sounds(svc);
	svc.controller_map.reset_triggers();
}

void FileMenu::frame_update(ServiceProvider& svc) {}

void FileMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(title);
	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});

	player->render(svc, win, svc::cameraLocator.get().physics.position);

	hud.render(*player, win);
}

} // namespace automa
