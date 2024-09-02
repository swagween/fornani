
#include "CreditsMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

CreditsMenu::CreditsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {
	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);
}

void CreditsMenu::init(ServiceProvider& svc, int room_number) {}

void CreditsMenu::handle_events(ServiceProvider& svc, sf::Event& event) {}

void CreditsMenu::tick_update(ServiceProvider& svc) {
	svc.controller_map.set_action_set(config::ActionSet::Menu);
	
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) {
		++current_selection;
		constrain_selection();
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) {
		--current_selection;
		constrain_selection();
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) {
		svc.state_controller.submenu = menu_type::options;
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	for (auto& option : options) { option.update(svc, current_selection); }
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection).left_offset);
	right_dot.set_target_position(options.at(current_selection).right_offset);

	svc.soundboard.play_sounds(svc);
}

void CreditsMenu::frame_update(ServiceProvider& svc) {}

void CreditsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {

	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
