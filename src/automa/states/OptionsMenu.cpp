
#include "OptionsMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

OptionsMenu::OptionsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {
	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);
}

void OptionsMenu::init(ServiceProvider& svc, std::string_view room) {}

void OptionsMenu::setTilesetTexture(ServiceProvider& svc, sf::Texture& t) {}

void OptionsMenu::handle_events(ServiceProvider& svc, sf::Event& event) {
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
	if (svc.controller_map.label_to_control.at("left").triggered() && !svc.controller_map.is_gamepad()) {
		svc.state_controller.submenu = menu_type::main;
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	if (!svc.controller_map.is_gamepad() && svc.controller_map.label_to_control.at("right").triggered()) {
		if (current_selection == menu_selection_id.at(MenuSelection::controls)) { svc.state_controller.submenu = menu_type::controls; }
		if (current_selection == menu_selection_id.at(MenuSelection::credits)) { svc.state_controller.submenu = menu_type::credits; }
		svc.state_controller.actions.set(Actions::trigger_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
	}
	if (svc.controller_map.label_to_control.at("main_action").triggered() || svc.controller_map.label_to_control.at("right").triggered()) {
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
	}
	if (svc.controller_map.label_to_control.at("secondary_action").triggered()) {
		svc.state_controller.submenu = menu_type::main;
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	svc.controller_map.reset_triggers();
}

void OptionsMenu::tick_update(ServiceProvider& svc) {
	for (auto& option : options) { option.update(svc, current_selection); }
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection).left_offset);
	right_dot.set_target_position(options.at(current_selection).right_offset);

	svc.soundboard.play_sounds(svc);
	svc.controller_map.reset_triggers();
}

void OptionsMenu::frame_update(ServiceProvider& svc) {}

void OptionsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	
	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
