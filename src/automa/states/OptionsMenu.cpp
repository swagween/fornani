
#include "OptionsMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

OptionsMenu::OptionsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {
	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);
}

void OptionsMenu::init(ServiceProvider& svc, int room_number) {}

void OptionsMenu::handle_events(ServiceProvider& svc, sf::Event& event) {}

void OptionsMenu::tick_update(ServiceProvider& svc) {
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
	if (svc.controller_map.label_to_control.at("left").triggered()) {
		svc.state_controller.submenu = menu_type::main;
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	if (svc.controller_map.label_to_control.at("right").triggered()) {
		if (current_selection == menu_selection_id.at(MenuSelection::controls)) { svc.state_controller.submenu = menu_type::controls; }
		if (current_selection == menu_selection_id.at(MenuSelection::credits)) { svc.state_controller.submenu = menu_type::credits; }
		if (current_selection == menu_selection_id.at(MenuSelection::settings)) { svc.state_controller.submenu = menu_type::settings; }
		svc.state_controller.actions.set(Actions::trigger_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
	}
	if (svc.controller_map.label_to_control.at("menu_forward").triggered() || svc.controller_map.label_to_control.at("right").triggered() || svc.controller_map.label_to_control.at("main_action").triggered()) {
		if (current_selection == menu_selection_id.at(MenuSelection::controls)) {
			svc.state_controller.submenu = menu_type::controls;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection == menu_selection_id.at(MenuSelection::credits)) {
			svc.state_controller.submenu = menu_type::credits;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection == menu_selection_id.at(MenuSelection::settings)) {
			svc.state_controller.submenu = menu_type::settings;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
	}
	if (svc.controller_map.label_to_control.at("menu_back").triggered()) {
		svc.state_controller.submenu = menu_type::main;
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

void OptionsMenu::frame_update(ServiceProvider& svc) {}

void OptionsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {

	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
