
#include "MainMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

MainMenu::MainMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {

	// playtester edition
	flags.set(GameStateFlags::playtest);
	// playtester edition

	svc.state_controller.actions.reset(Actions::intro_done);

	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);

	if (flags.test(GameStateFlags::playtest)) { subtitle.setString("Playtester Edition"); }
	subtitle.setLineSpacing(1.5f);
	subtitle.setFont(subtitle_font);
	subtitle.setLetterSpacing(1.2f);
	subtitle.setCharacterSize(options.at(current_selection).label.getCharacterSize());
	subtitle.setPosition(svc.constants.screen_dimensions.x * 0.5f - subtitle.getLocalBounds().width * 0.5f, svc.constants.screen_dimensions.y - 324.f);
	subtitle.setFillColor(svc.styles.colors.red);
	if (flags.test(GameStateFlags::playtest)) { instruction.setString("press [P] to open playtester portal"); }
	instruction.setLineSpacing(1.5f);
	instruction.setFont(subtitle_font);
	instruction.setLetterSpacing(1.2f);
	instruction.setCharacterSize(options.at(current_selection).label.getCharacterSize());
	instruction.setPosition(svc.constants.screen_dimensions.x * 0.5f - instruction.getLocalBounds().width * 0.5f, svc.constants.screen_dimensions.y - 36.f);
	instruction.setFillColor(svc.styles.colors.dark_grey);

	title = sf::Sprite{svc.assets.t_title, sf::IntRect({0, 0}, {(int)svc.constants.screen_dimensions.x, (int)svc.constants.screen_dimensions.y})};

	svc.data.load_blank_save(player);
	player.controller.autonomous_walk();
	player.set_position({(float)(svc.constants.screen_dimensions.x / 2) + 80, 360});
	player.antennae.at(0).set_position({(float)(svc.constants.screen_dimensions.x / 2) + 80, 360});
	player.antennae.at(1).set_position({(float)(svc.constants.screen_dimensions.x / 2) + 80, 360});

	svc.music.load("clay");
	svc.music.play_looped(20);
};

void MainMenu::init(ServiceProvider& svc, std::string_view room) {
}

void MainMenu::handle_events(ServiceProvider& svc, sf::Event& event) {
	svc.controller_map.handle_mouse_events(event);
	svc.controller_map.handle_joystick_events(event);
	if (event.type == sf::Event::EventType::KeyPressed) { svc.controller_map.handle_press(event.key.code); }
	if (event.type == sf::Event::EventType::KeyReleased) { svc.controller_map.handle_release(event.key.code); }

	if (svc.controller_map.label_to_control.at("down").triggered()) {
		++current_selection;
		constrain_selection();
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.label_to_control.at("up").triggered()) {
		--current_selection;
		constrain_selection();
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.label_to_control.at("menu_forward").triggered()) {
		if (current_selection == menu_selection_id.at(MenuSelection::play)) {
			svc.state_controller.submenu = menu_type::file_select;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection == menu_selection_id.at(MenuSelection::options)) {
			svc.state_controller.submenu = menu_type::options;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection == menu_selection_id.at(MenuSelection::quit)) {
			svc.state_controller.actions.set(Actions::shutdown);
		}
	}
	if (!svc.controller_map.is_gamepad() && svc.controller_map.label_to_control.at("right").triggered()) {
		if (current_selection == menu_selection_id.at(MenuSelection::play)) { svc.state_controller.submenu = menu_type::file_select; }
		if (current_selection == menu_selection_id.at(MenuSelection::options)) { svc.state_controller.submenu = menu_type::options; }
		svc.state_controller.actions.set(Actions::trigger_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
	}
	svc.controller_map.reset_triggers();
}

void MainMenu::tick_update(ServiceProvider& svc) {
	for (auto& option : options) { option.update(svc, current_selection); }
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection).left_offset);
	right_dot.set_target_position(options.at(current_selection).right_offset);
	svc.soundboard.play_sounds(svc);
	svc.controller_map.reset_triggers();
	player->animation.state = player::AnimState::run;
}

void MainMenu::frame_update(ServiceProvider& svc) {}

void MainMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(title);
	win.draw(subtitle);
	win.draw(instruction);
	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
