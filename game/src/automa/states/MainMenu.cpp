
#include "fornani/automa/states/MainMenu.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

MainMenu::MainMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number)
	: GameState(svc, player, scene, room_number), subtitle{svc.text.fonts.basic}, instruction(svc.text.fonts.basic), title(svc.assets.t_title) {
	subtitle.setFont(svc.text.fonts.basic);
	// playtester edition
	flags.set(GameStateFlags::playtest);
	// playtester edition
	svc.app_flags.reset(AppFlags::in_game);
	svc.state_controller.actions.reset(Actions::intro_done);

	left_dot.set_position(options.at(current_selection.get()).left_offset);
	right_dot.set_position(options.at(current_selection.get()).right_offset);

	if (flags.test(GameStateFlags::playtest)) { subtitle.setString(svc.version->version_title()); }
	subtitle.setLineSpacing(1.5f);
	subtitle.setLetterSpacing(1.2f);
	subtitle.setCharacterSize(options.at(current_selection.get()).label.getCharacterSize());
	subtitle.setPosition({svc.constants.f_center_screen.x - subtitle.getLocalBounds().getCenter().x, svc.constants.screen_dimensions.y - 300.f});
	subtitle.setFillColor(svc.styles.colors.red);
	if (flags.test(GameStateFlags::playtest)) { instruction.setString("press [P] to open playtester portal"); }
	instruction.setLineSpacing(1.5f);
	instruction.setLetterSpacing(1.2f);
	instruction.setCharacterSize(options.at(current_selection.get()).label.getCharacterSize());
	instruction.setPosition({svc.constants.screen_dimensions.x * 0.5f - instruction.getLocalBounds().getCenter().x, svc.constants.screen_dimensions.y - 36.f});
	instruction.setFillColor(svc.styles.colors.dark_grey);

	svc.data.load_blank_save(player);
	player.controller.autonomous_walk();
	player.set_position({svc.constants.f_center_screen.x + 80.f, 360.f});
	loading.start();
}

void MainMenu::tick_update(ServiceProvider& svc) {
	svc.a11y.set_action_ctx_bar_enabled(true);

	if (loading.is_almost_complete()) {
		svc.music.load(svc.finder, "clay");
		svc.music.play_looped(20);
	}
	loading.update();

	svc.controller_map.set_action_set(config::ActionSet::Menu);

	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) {
		current_selection.modulate(1);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) {
		current_selection.modulate(-1);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
		if (current_selection.get() == menu_selection_id.at(MenuSelection::play)) {
			svc.state_controller.submenu = menu_type::file_select;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection.get() == menu_selection_id.at(MenuSelection::options)) {
			svc.state_controller.submenu = menu_type::options;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection.get() == menu_selection_id.at(MenuSelection::quit)) { svc.state_controller.actions.set(Actions::shutdown); }
	}

	for (auto& option : options) { option.update(svc, current_selection.get()); }

	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection.get()).left_offset);
	right_dot.set_target_position(options.at(current_selection.get()).right_offset);
	svc.soundboard.play_sounds(svc);
	player->animation.state = player::AnimState::run;
}

void MainMenu::frame_update(ServiceProvider& svc) {}

void MainMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(title);
	win.draw(subtitle);
	// win.draw(instruction);
	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
