
#include "fornani/automa/states/OptionsMenu.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

OptionsMenu::OptionsMenu(ServiceProvider& svc, player::Player& player) : MenuState(svc, player, "options") { m_parent_menu = MenuType::main; }

void OptionsMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	MenuState::tick_update(svc, engine);

	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
		if (current_selection.get() == menu_selection_id.at(MenuSelection::credits)) {
			svc.state_controller.submenu = MenuType::credits;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection.get() == menu_selection_id.at(MenuSelection::controls)) {
			svc.state_controller.submenu = MenuType::controls;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection.get() == menu_selection_id.at(MenuSelection::settings)) {
			svc.state_controller.submenu = MenuType::settings;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
	}

	for (auto& option : options) { option.update(svc, current_selection.get()); }
}

void OptionsMenu::frame_update(ServiceProvider& svc) {}

void OptionsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) { MenuState::render(svc, win); }

} // namespace fornani::automa
