
#include "fornani/automa/states/OptionsMenu.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/WindowManager.hpp"

namespace fornani::automa {

OptionsMenu::OptionsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number) : GameState(svc, player, scene, room_number) {
	left_dot.set_position(options.at(current_selection.get()).left_offset);
	right_dot.set_position(options.at(current_selection.get()).right_offset);
}

void OptionsMenu::tick_update(ServiceProvider& svc) {
	GameState::tick_update(svc);
	svc.controller_map.set_action_set(config::ActionSet::Menu);

	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) {
		current_selection.modulate(1);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) {
		current_selection.modulate(-1);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) {
		svc.state_controller.submenu = MenuType::main;
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
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
		if (current_selection.get() == menu_selection_id.at(MenuSelection::credits)) {
			svc.state_controller.submenu = MenuType::credits;
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

	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection.get()).left_offset);
	right_dot.set_target_position(options.at(current_selection.get()).right_offset);

	svc.soundboard.play_sounds(svc);
}

void OptionsMenu::frame_update(ServiceProvider& svc) {}

void OptionsMenu::render(ServiceProvider& svc, WindowManager& win) {

	for (auto& option : options) { win.get().draw(option.label); }

	left_dot.render(svc, win.get(), {0, 0});
	right_dot.render(svc, win.get(), {0, 0});
}

} // namespace fornani::automa
