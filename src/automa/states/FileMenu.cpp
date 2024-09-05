
#include "FileMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

FileMenu::FileMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id), map(svc, player, console), file_select_menu(svc, {"play", "stats", "delete"}) {
	current_selection = util::Circuit(num_files);
	svc.data.load_blank_save(player);
	console = gui::Console(svc);
	console.set_source(svc.text.basic);
	hud.set_corner_pad(svc, true); // display hud preview for each file in the center of the screen
	svc.state_controller.next_state = svc.data.load_progress(player, current_selection.get());
	player.set_position({(float)(svc.constants.screen_dimensions.x / 2) + 80, 360});
	player.antennae.at(0).set_position({(float)(svc.constants.screen_dimensions.x / 2) + 80, 360});
	player.antennae.at(1).set_position({(float)(svc.constants.screen_dimensions.x / 2) + 80, 360});
	player.hurt_cooldown.cancel();

	loading.start(4);

	title.setPosition(0, 0);
	title.setSize(static_cast<sf::Vector2f>(svc.constants.screen_dimensions));
	title.setFillColor(svc.styles.colors.ui_black);

	refresh(svc);

	left_dot.set_position(options.at(current_selection.get()).left_offset);
	right_dot.set_position(options.at(current_selection.get()).right_offset);
}

void FileMenu::init(ServiceProvider& svc, int room_number) {}

void FileMenu::handle_events(ServiceProvider& svc, sf::Event& event) {
	svc.controller_map.handle_mouse_events(event);
	svc.controller_map.handle_joystick_events(event);
	if (event.type == sf::Event::EventType::KeyPressed) { svc.controller_map.handle_press(event.key.code); }
	if (event.type == sf::Event::EventType::KeyReleased) { svc.controller_map.handle_release(event.key.code); }
	if (console.active()) { return; }
	if (svc.controller_map.label_to_control.at("down").triggered()) {
		if (file_select_menu.is_open()) {
			file_select_menu.down();
		} else {
			current_selection.modulate(1);
			svc.data.load_blank_save(*player);
			svc.state_controller.next_state = svc.data.load_progress(*player, current_selection.get());
		}
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.label_to_control.at("up").triggered()) {
		if (file_select_menu.is_open()) {
			file_select_menu.up();
		} else {
			current_selection.modulate(-1);
			svc.data.load_blank_save(*player);
			svc.state_controller.next_state = svc.data.load_progress(*player, current_selection.get());
		}
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.label_to_control.at("left").triggered() && !svc.controller_map.is_gamepad()) {
		if (file_select_menu.is_open()) {
			file_select_menu.close(svc);
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		} else {
			svc.state_controller.actions.set(Actions::exit_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		}
	}
	if (svc.controller_map.label_to_control.at("menu_forward").triggered() || svc.controller_map.label_to_control.at("main_action").triggered() ||
		(svc.controller_map.label_to_control.at("right").triggered() && !svc.controller_map.is_gamepad())) {
		if (file_select_menu.is_open()) {
			switch (file_select_menu.get_selection()) {
			case 0:
				svc.state_controller.next_state = svc.data.load_progress(*player, current_selection.get(), true);
				svc.state_controller.actions.set(Actions::trigger);
				svc.state_controller.actions.set(Actions::save_loaded);
				svc.soundboard.flags.menu.set(audio::Menu::select);
				svc.soundboard.flags.world.set(audio::World::load);
				break;
			case 1:
				svc.state_controller.actions.set(automa::Actions::print_stats);
				svc.state_controller.actions.set(Actions::trigger);
				svc.soundboard.flags.menu.set(audio::Menu::select);
				break;
			case 2:
				console.load_and_launch("delete_file");
				file_select_menu.close(svc);
				break;
			}
		} else {
			file_select_menu.open(svc, options.at(current_selection.get()).position);
			svc.soundboard.flags.console.set(audio::Console::menu_open);
		}
	}
	if (svc.controller_map.label_to_control.at("menu_back").triggered() || svc.controller_map.label_to_control.at("menu_toggle_secondary").triggered()) {
		if (file_select_menu.is_open()) {
			file_select_menu.close(svc);
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		} else {
			svc.state_controller.submenu = menu_type::main;
			svc.state_controller.actions.set(Actions::exit_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		}
	}
	svc.controller_map.reset_triggers();
}

void FileMenu::tick_update(ServiceProvider& svc) {
	for (auto& option : options) { option.update(svc, current_selection.get()); }

	// file deletion requested
	if (svc.state_controller.actions.consume(Actions::delete_file)) {
		svc.data.delete_file(current_selection.get());
		refresh(svc);
		svc.state_controller.next_state = svc.data.load_progress(*player, current_selection.get());
		std::cout << current_selection.get() << "\n";
	}

	auto& opt = options.at(current_selection.get());
	auto minimenu_dim = sf::Vector2<float>{128.f, 128.f};
	auto minimenu_pos = opt.position + sf::Vector2<float>(opt.label.getLocalBounds().width * 0.5f + minimenu_dim.x * 0.5f + 2.f * spacing, 0.f);
	file_select_menu.update(svc, minimenu_dim, minimenu_pos);

	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection.get()).left_offset);
	right_dot.set_target_position(options.at(current_selection.get()).right_offset);

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

	console.update(svc);

	loading.update();

	svc.controller_map.reset_triggers();
	player->controller.clean();
	svc.soundboard.play_sounds(svc);
	player->flags.triggers = {};
	console.end_tick();
}

void FileMenu::frame_update(ServiceProvider& svc) {}

void FileMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	if (!loading.is_complete()) { return; }
	win.draw(title);
	for (auto& option : options) { win.draw(option.label); }
	player->render(svc, win, {});
	if (loading.is_complete()) {
		left_dot.render(svc, win, {});
		right_dot.render(svc, win, {});
		hud.render(*player, win);
		file_select_menu.render(win);
	}
	if (console.flags.test(gui::ConsoleFlags::active)) { console.render(win); }
	console.write(win, false);
}

void FileMenu::refresh(ServiceProvider& svc) {
	auto ctr{0};
	for (auto& save : svc.data.files) {
		if (save.is_new() && options.at(ctr).label.getString().getSize() < 8) { options.at(ctr).label.setString(options.at(ctr).label.getString() + " (new)"); }
		++ctr;
	}
	for (auto& option : options) { option.update(svc, current_selection.get()); }
}

} // namespace automa
