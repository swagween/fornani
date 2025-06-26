
#include "fornani/automa/states/CreditsMenu.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

CreditsMenu::CreditsMenu(ServiceProvider& svc, player::Player& player, std::string_view room_name, int room_number) : GameState(svc, player, room_name, room_number) {
	left_dot.set_position(options.at(current_selection.get()).left_offset);
	right_dot.set_position(options.at(current_selection.get()).right_offset);
}

void CreditsMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	GameState::tick_update(svc, engine);
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
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	for (auto& option : options) { option.update(svc, current_selection.get()); }
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection.get()).left_offset);
	right_dot.set_target_position(options.at(current_selection.get()).right_offset);

	svc.soundboard.play_sounds(engine, svc);
}

void CreditsMenu::frame_update(ServiceProvider& svc) {}

void CreditsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {

	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace fornani::automa
