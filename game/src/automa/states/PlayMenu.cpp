
#include "fornani/automa/states/PlayMenu.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

PlayMenu::PlayMenu(ServiceProvider& svc, player::Player& player) : MenuState(svc, player, "play") { m_parent_menu = MenuType::main; }

void PlayMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	MenuState::tick_update(svc, engine);
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
		if (current_selection.get() == 0) {
			svc.state_controller.submenu = MenuType::file_select;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection.get() == 1) {
			svc.state_controller.submenu = MenuType::trials;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
	}
	for (auto& option : options) { option.update(svc, current_selection.get()); }
}

void PlayMenu::frame_update(ServiceProvider& svc) {}

void PlayMenu::render(ServiceProvider& svc, sf::RenderWindow& win) { MenuState::render(svc, win); }

} // namespace fornani::automa
