
#include "FileMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

FileMenu::FileMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id), map(svc, player, console) {
	current_selection = 0;
	svc.data.load_blank_save(player);
	hud.set_corner_pad(svc, true); // display hud preview for each file in the center of the screen
	constrain_selection();
	svc.state_controller.next_state = svc.data.load_progress(player, current_selection);
	player.set_position({(float)(svc.constants.screen_dimensions.x / 2) + 80, 360});
	player.antennae.at(0).set_position({(float)(svc.constants.screen_dimensions.x / 2) + 80, 360});
	player.antennae.at(1).set_position({(float)(svc.constants.screen_dimensions.x / 2) + 80, 360});

	loading.start(1);

	title.setPosition(0, 0);
	title.setSize(static_cast<sf::Vector2f>(svc.constants.screen_dimensions));
	title.setFillColor(svc.styles.colors.ui_black);

	auto ctr{0};
	for (auto& save : svc.data.files) {
		if (save.is_new()) { options.at(ctr).label.setString(options.at(ctr).label.getString() + " (new)"); }
		++ctr;
	}
	for (auto& option : options) { option.update(svc, current_selection); }

	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);
}

void FileMenu::init(ServiceProvider& svc, std::string_view room) {}

void FileMenu::handle_events(ServiceProvider& svc, sf::Event& event) {
	svc.controller_map.handle_mouse_events(event);
	svc.controller_map.handle_joystick_events(event);
	if (event.type == sf::Event::EventType::KeyPressed) { svc.controller_map.handle_press(event.key.code); }
	if (event.type == sf::Event::EventType::KeyReleased) { svc.controller_map.handle_release(event.key.code); }

	if (svc.controller_map.label_to_control.at("down").triggered()) {
		++current_selection;
		constrain_selection();
		svc.data.load_blank_save(*player);
		svc.state_controller.next_state = svc.data.load_progress(*player, current_selection);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.label_to_control.at("up").triggered()) {
		--current_selection;
		constrain_selection();
		svc.data.load_blank_save(*player);
		svc.state_controller.next_state = svc.data.load_progress(*player, current_selection);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.label_to_control.at("left").triggered() && !svc.controller_map.is_gamepad()) {
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	if (svc.controller_map.label_to_control.at("menu_forward").triggered()) {
		constrain_selection();
		svc.state_controller.next_state = svc.data.load_progress(*player, current_selection, true);
		svc.state_controller.actions.set(Actions::trigger);
		svc.state_controller.actions.set(Actions::save_loaded);
		svc.soundboard.flags.menu.set(audio::Menu::select);
		svc.soundboard.flags.world.set(audio::World::load);
	}
	if (svc.controller_map.label_to_control.at("menu_back").triggered()) {
		svc.state_controller.submenu = menu_type::main;
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
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

	hud.update(svc, *player);

	player->animation.state = player::AnimState::run;
	player->collider.physics.acceleration = {};
	player->collider.physics.velocity = {};
	player->collider.physics.zero();
	player->collider.reset();
	player->controller.autonomous_walk();
	player->collider.flags.state.set(shape::State::grounded);

	player->set_position({svc.constants.screen_dimensions.x * 0.5f + 80, 360});
	player->update(map, console, inventory_window);
	player->controller.direction.lr = dir::LR::left;
	svc.soundboard.flags.player.reset(audio::Player::step);

	loading.update();

	svc.soundboard.play_sounds(svc);
	svc.controller_map.reset_triggers();
}

void FileMenu::frame_update(ServiceProvider& svc) {}

void FileMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(title);
	for (auto& option : options) { win.draw(option.label); }
	player->render(svc, win, {});
	if (loading.is_complete()) {
		left_dot.render(svc, win, {});
		right_dot.render(svc, win, {});
		hud.render(*player, win);
	}
}

} // namespace automa
