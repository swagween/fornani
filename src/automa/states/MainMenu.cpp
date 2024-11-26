
#include "MainMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

MainMenu::MainMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id), option_list(svc) {

	// playtester edition
	flags.set(GameStateFlags::playtest);
	// playtester edition
	svc.app_flags.reset(AppFlags::in_game);
	svc.state_controller.actions.reset(Actions::intro_done);

	if (flags.test(GameStateFlags::playtest)) { subtitle.setString("Playtester Edition"); }
	subtitle.setLineSpacing(1.5f);
	subtitle.setFont(subtitle_font);
	subtitle.setLetterSpacing(1.2f);
	subtitle.setCharacterSize(16.f);
	subtitle.setPosition(svc.constants.screen_dimensions.x * 0.5f - subtitle.getLocalBounds().width * 0.5f, svc.constants.screen_dimensions.y - 300.f);
	subtitle.setFillColor(svc.styles.colors.red);
	if (flags.test(GameStateFlags::playtest)) { instruction.setString("press [P] to open playtester portal"); }
	instruction.setLineSpacing(1.5f);
	instruction.setFont(subtitle_font);
	instruction.setLetterSpacing(1.2f);
	instruction.setCharacterSize(16.f);
	instruction.setPosition(svc.constants.screen_dimensions.x * 0.5f - instruction.getLocalBounds().width * 0.5f, svc.constants.screen_dimensions.y - 36.f);
	instruction.setFillColor(svc.styles.colors.dark_grey);

	title = sf::Sprite{svc.assets.t_title, sf::IntRect({0, 0}, {(int)svc.constants.screen_dimensions.x, (int)svc.constants.screen_dimensions.y})};

	option_list.push_option("play", [&svc]() {
		svc.state_controller.submenu = menu_type::file_select;
		svc.state_controller.actions.set(Actions::trigger_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
	});
	option_list.push_option("options", [&svc]() {
		svc.state_controller.submenu = menu_type::options;
		svc.state_controller.actions.set(Actions::trigger_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
	});
	option_list.push_option("exit", [&svc]() { svc.state_controller.actions.set(Actions::shutdown); });

	svc.data.load_blank_save(player);
	player.controller.autonomous_walk();
	player.set_position({(float)(svc.constants.screen_dimensions.x / 2) + 80, 360});
	player.antennae.at(0).set_position({(float)(svc.constants.screen_dimensions.x / 2) + 80, 360});
	player.antennae.at(1).set_position({(float)(svc.constants.screen_dimensions.x / 2) + 80, 360});
	loading.start();
};

void MainMenu::init(ServiceProvider& svc, int room_number) {}

void MainMenu::handle_events(ServiceProvider& svc, sf::Event& event) {}

void MainMenu::tick_update(ServiceProvider& svc) {
	svc.a11y.set_action_ctx_bar_enabled(true);

	if (loading.is_almost_complete()) {
		svc.music.load("clay");
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
	option_list.update(svc);

	svc.soundboard.play_sounds(svc);
	player->animation.state = player::AnimState::run;
}

void MainMenu::frame_update(ServiceProvider& svc) {}

void MainMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(title);
	win.draw(subtitle);
	// win.draw(instruction);
	option_list.render(svc, win, {0.f, 80.f});
}

} // namespace automa
