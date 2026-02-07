
#include "fornani/automa/states/MainMenu.hpp"
#include "fornani/graphics/Colors.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Constants.hpp"

namespace fornani::automa {

MainMenu::MainMenu(ServiceProvider& svc, player::Player& player) : MenuState(svc, player, "main"), subtitle{svc.text.fonts.basic}, instruction(svc.text.fonts.basic), title(svc.assets.get_texture("menu_title")) {
	subtitle.setFont(svc.text.fonts.basic);
	// playtester edition
	flags.set(GameStateFlags::playtest);
	// playtester edition
	svc.app_flags.reset(AppFlags::in_game);
	svc.state_controller.actions.reset(Actions::intro_done);

	title.setTextureRect(sf::IntRect{{0, 10 * p_theme.title_index}, {47, 10}});
	title.setPosition({362.f, 161.f});
	title.setScale({5.f, 5.f});

	if (flags.test(GameStateFlags::playtest)) { subtitle.setString(svc.version->version_title()); }
	subtitle.setLineSpacing(1.5f);
	subtitle.setLetterSpacing(1.2f);
	subtitle.setCharacterSize(options.at(current_selection.get()).label.getCharacterSize());
	subtitle.setPosition({svc.window->f_center_screen().x - subtitle.getLocalBounds().getCenter().x, svc.window->i_screen_dimensions().y - 300.f});
	subtitle.setFillColor(p_theme.activated_text_color);
	if (flags.test(GameStateFlags::playtest)) { instruction.setString("press [P] to open playtester portal"); }
	instruction.setLineSpacing(1.5f);
	instruction.setLetterSpacing(1.2f);
	instruction.setCharacterSize(options.at(current_selection.get()).label.getCharacterSize());
	instruction.setPosition({svc.window->i_screen_dimensions().x * 0.5f - instruction.getLocalBounds().getCenter().x, svc.window->i_screen_dimensions().y - 36.f});
	instruction.setFillColor(p_theme.deactivated_text_color);

	svc.data.load_blank_save(player);
	svc.ambience_player.load(svc.finder, "none");
	svc.ambience_player.play();
	loading.start();
	svc.ambience_player.set_balance(1.f);
	svc.music_player.set_balance(0.f);
}

void MainMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	MenuState::tick_update(svc, engine);
	svc.a11y.set_action_ctx_bar_enabled(true);

	if (loading.is_almost_complete()) {
		svc.music_player.load(svc.finder, "clay");
		svc.music_player.play_looped();
	}
	loading.update();
	if (svc.input_system.digital(input::DigitalAction::menu_select).triggered) {
		NANI_LOG_DEBUG(m_logger, "Menu Select Triggered.");
		if (current_selection.get() == menu_selection_id.at(MenuSelection::play)) {
			svc.state_controller.submenu = MenuType::play;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection.get() == menu_selection_id.at(MenuSelection::options)) {
			svc.state_controller.submenu = MenuType::options;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection.get() == menu_selection_id.at(MenuSelection::quit)) { svc.state_controller.actions.set(Actions::shutdown); }
	}
}

void MainMenu::frame_update(ServiceProvider& svc) {}

void MainMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	MenuState::render(svc, win);
	win.draw(title);
	win.draw(subtitle);
}

} // namespace fornani::automa
