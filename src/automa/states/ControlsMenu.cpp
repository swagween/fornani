
#include "ControlsMenu.hpp"
#include <algorithm>
#include "../../service/ServiceProvider.hpp"

namespace automa {

// XXX clean this up, use function from controllermap?
auto get_action_by_identifier(std::string_view id) -> config::DigitalAction {
	static std::unordered_map<std::string_view, config::DigitalAction> const map = {
		{"platformer_left", config::DigitalAction::platformer_left},
		{"platformer_right", config::DigitalAction::platformer_right},
		{"platformer_up", config::DigitalAction::platformer_up},
		{"platformer_down", config::DigitalAction::platformer_down},
		{"platformer_jump", config::DigitalAction::platformer_jump},
		{"platformer_shoot", config::DigitalAction::platformer_shoot},
		{"platformer_sprint", config::DigitalAction::platformer_sprint},
		{"platformer_shield", config::DigitalAction::platformer_shield},
		{"platformer_inspect", config::DigitalAction::platformer_inspect},
		{"platformer_switch weapon (left)", config::DigitalAction::platformer_arms_switch_left},
		{"platformer_switch weapon (right)", config::DigitalAction::platformer_arms_switch_right},
		{"platformer_open inventory", config::DigitalAction::platformer_open_inventory},
		{"platformer_open map", config::DigitalAction::platformer_open_map},
		{"platformer_pause", config::DigitalAction::platformer_toggle_pause},
		{"inventory_left", config::DigitalAction::inventory_left},
		{"inventory_right", config::DigitalAction::inventory_right},
		{"inventory_up", config::DigitalAction::inventory_up},
		{"inventory_down", config::DigitalAction::inventory_down},
		{"inventory_close", config::DigitalAction::inventory_close},
		{"map_close", config::DigitalAction::map_close},
		{"menu_up", config::DigitalAction::menu_up},
		{"menu_down", config::DigitalAction::menu_down},
		{"menu_select", config::DigitalAction::menu_select},
		{"menu_cancel", config::DigitalAction::menu_cancel},
	};

	return map.at(id);
}

constexpr std::array<std::string_view, 4> tabs = {"controls_platformer", "controls_inventory", "controls_map", "controls_menu"};
constexpr std::array<std::string_view, 4> tab_id_prefixes = {"platformer_", "inventory_", "map_", "menu_"};

ControlsMenu::ControlsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {
	change_scene(svc, "controls_platformer");
	refresh_controls(svc);
	instruction.setLineSpacing(1.5f);
	instruction.setFont(font);
	instruction.setLetterSpacing(title_letter_spacing);
	instruction.setCharacterSize(options.at(current_selection).label.getCharacterSize());
	instruction.setPosition(svc.constants.screen_dimensions.x * 0.5f - instruction.getLocalBounds().width * 0.5f, svc.constants.screen_dimensions.y - 120.f);
	instruction.setFillColor(svc.styles.colors.dark_grey);

	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);
	loading.start(2);

	debug.setFillColor(sf::Color::Transparent);
	debug.setSize({2.f, (float)svc.constants.screen_dimensions.y});
	debug.setOutlineColor(svc.styles.colors.blue);
	debug.setOutlineThickness(-1);
}

void ControlsMenu::init(ServiceProvider& svc, int room_number) {}

void ControlsMenu::handle_events(ServiceProvider& svc, sf::Event& event) {
	if (option_is_selected && current_selection > 0 && current_selection < options.size() - 2) {
		// Currently binding key
		if (event.type == sf::Event::EventType::KeyPressed) {
			auto current_tab = std::distance(tabs.begin(), std::find(tabs.begin(), tabs.end(), scene));
			auto id = std::string(tab_id_prefixes.at(current_tab)) + static_cast<std::string>(options.at(current_selection).label.getString());
			auto action = get_action_by_identifier(id.data());

			svc.controller_map.set_primary_keyboard_binding(action, event.key.code);
			option_is_selected = false;
			refresh_controls(svc);
		}
	}
}

void ControlsMenu::tick_update(ServiceProvider& svc) {
	svc.controller_map.set_action_set(config::ActionSet::Menu);

	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) {
		if (option_is_selected) {
			// Action set selection should be first option
			if (current_selection == 0) {
				auto current_tab = std::distance(tabs.begin(), std::find(tabs.begin(), tabs.end(), scene));
				auto tab_to_switch_to = (current_tab + 1) % 4;
				change_scene(svc, tabs[tab_to_switch_to]);
			}
		} else {
			++current_selection;
			constrain_selection();
			svc.soundboard.flags.menu.set(audio::Menu::shift);
		}
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) {
		if (option_is_selected) {
			// Action set selection should be first option
			if (current_selection == 0) {
				auto current_tab = std::distance(tabs.begin(), std::find(tabs.begin(), tabs.end(), scene));
				if (current_tab == 0) { current_tab = 4; }
				auto tab_to_switch_to = current_tab - 1;
				change_scene(svc, tabs[tab_to_switch_to]);
			}
		} else {
			--current_selection;
			constrain_selection();
			svc.soundboard.flags.menu.set(audio::Menu::shift);
		}
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) {
		if (option_is_selected) {
			option_is_selected = false;
		} else {
			svc.state_controller.submenu = menu_type::options;
			svc.state_controller.actions.set(Actions::exit_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		}
	}
	auto pressed_select = svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered;
	if (pressed_select) {
		// Gamepad settings should be second to last option
		if (current_selection == options.size() - 2 && svc.controller_map.gamepad_connected()) {
			svc.controller_map.open_bindings_overlay();
		}
		// Reset to default should be last option
		else if (current_selection == options.size() - 1) {
			restore_defaults(svc);
		} else {
			option_is_selected = !option_is_selected;
			auto& control = control_list.at(current_selection);
			control.setString("Press a key");
			control.setOrigin(control.getLocalBounds().width, control.getLocalBounds().height * 0.5f);
		}
	}

	for (auto& option : options) {
		option.flagged = current_selection == option.index && option_is_selected;
		option.update(svc, current_selection);
	}

	loading.update();
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection).left_offset);
	right_dot.set_target_position(options.at(current_selection).right_offset);

	svc.soundboard.play_sounds(svc);
}

void ControlsMenu::frame_update(ServiceProvider& svc) {}

void ControlsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {

	if (loading.is_complete()) {
		for (auto& option : options) { win.draw(option.label); }
		for (auto& control : control_list) { win.draw(control); }
		win.draw(instruction);
	}

	left_dot.render(svc, win, {});
	right_dot.render(svc, win, {});
}

void ControlsMenu::refresh_controls(ServiceProvider& svc) {
	// svc.data.load_controls(svc.controller_map);
	size_t ctr{0};

	for (auto& option : options) {
		if (ctr > 0 && ctr < options.size() - 2) {
			auto current_tab = std::distance(tabs.begin(), std::find(tabs.begin(), tabs.end(), scene));
			auto id = std::string(tab_id_prefixes.at(current_tab)) + static_cast<std::string>(option.label.getString());
			auto action = get_action_by_identifier(id.data());

			auto& control = control_list.at(ctr);
			control.setString(std::string(svc.controller_map.key_to_string(svc.controller_map.get_primary_keyboard_binding(action))));
			control.setOrigin(control.getLocalBounds().width, control.getLocalBounds().height * 0.5f);
			control.setPosition(svc.constants.screen_dimensions.x * 0.5f + center_offset, option.position.y);
			control.setCharacterSize(16);
			control.setLetterSpacing(title_letter_spacing);
			control.setFillColor(svc.styles.colors.dark_grey);
			control.setOrigin(control.getLocalBounds().width * 0.5f, control.getLocalBounds().height * 0.5f);
		}
		++ctr;
	}
	// XXX
	// instruction.setString("TODO TEXT");
	// svc.controller_map.is_gamepad() ? instruction.setString("current controller : GAMEPAD\npress [START] to change binding, and [Esc] to cancel\npress [Ctrl] to restore defaults")
	// 								: instruction.setString("current controller : KEYBOARD\npress [ENTER] to change binding, and [Esc] to cancel\npress [Ctrl] to restore defaults");
}

void ControlsMenu::update_binding(ServiceProvider& svc, sf::Event& event) {
	return; // XXX
	// XXX if (current_selection >= options.size() || current_selection >= control_list.size() || current_selection >= svc.controller_map.tags.size()) { return; }
	options.at(current_selection).label.setFillColor(svc.styles.colors.bright_orange);
	control_list.at(current_selection).setFillColor(svc.styles.colors.bright_orange);
	if (true) { // XXX svc.controller_map.is_keyboard()
		if (event.type == sf::Event::KeyPressed) {
			option_is_selected = false;
			/* XXX
			std::string_view tag = svc.controller_map.tags.at(current_selection);
			svc.data.controls["controls"][tag]["mouse_button"] = "";
			if (!svc.controller_map.key_to_string.contains(event.key.code)) { return; }
			svc.data.controls["controls"][tag]["keyboard_key"] = svc.controller_map.key_to_string.at(event.key.code);
			svc.data.save_controls(svc.controller_map);
			refresh_controls(svc); */
		}
		if (event.type == sf::Event::MouseButtonPressed) {
			option_is_selected = false;
			/* XXX
			std::string_view tag = svc.controller_map.tags.at(current_selection);
			svc.data.controls["controls"][tag]["keyboard_key"] = "";
			if (event.mouseButton.button == sf::Mouse::Left) { svc.data.controls["controls"][tag]["mouse_button"] = "LMB"; }
			if (event.mouseButton.button == sf::Mouse::Right) { svc.data.controls["controls"][tag]["mouse_button"] = "RMB"; }
			svc.data.save_controls(svc.controller_map);
			refresh_controls(svc); */
		}
	}
	if (false) { // XXX svc.controller_map.is_gamepad()
		if (event.type == sf::Event::JoystickButtonPressed) {
			option_is_selected = false;
			/* XXX
			std::string_view tag = svc.controller_map.tags.at(current_selection);
			svc.data.controls["controls"][tag]["gamepad_button"] = event.joystickButton.button;
			svc.data.save_controls(svc.controller_map);
			refresh_controls(svc); */
		}
	}
}

void ControlsMenu::restore_defaults(ServiceProvider& svc) {
	svc.data.reset_controls();
	svc.data.save_controls(svc.controller_map);
	svc.data.load_controls(svc.controller_map);
	refresh_controls(svc);
}

void ControlsMenu::change_scene(ServiceProvider& svc, std::string_view to_change_to) {
	scene = to_change_to;

	options.clear();
	control_list.clear();
	auto const& in_data = svc.data.menu["options"];
	for (auto& entry : in_data[scene].array_view()) { options.push_back(Option(svc, entry.as_string())); }
	top_buffer = svc.data.menu["config"][scene]["top_buffer"].as<float>();
	int ctr{};
	for (auto& option : options) {
		option.label.setFont(font);
		if (ctr == 0 || ctr >= options.size() - 2) {
			// Show tab selector, gamepad settings & reset to default in middle of screen
			option.position.x = svc.constants.screen_dimensions.x * 0.5f;
		} else {
			option.position.x = svc.constants.screen_dimensions.x * 0.5f - center_offset + option.label.getLocalBounds().width * 0.5f;
		}
		// FIXME Spacing is broken in other menus. getLocalBounds().height is returning 0 because the font isn't set when the function is called
		// 	     To make up for it we don't add the getLocalBounds().height factor here, but keep it in mind when it is fixed!
		option.position.y = top_buffer + ctr * (spacing);
		option.index = ctr;
		option.update(svc, current_selection);

		control_list.push_back(sf::Text());
		control_list.back().setFont(font);
		++ctr;
	}

	refresh_controls(svc);
}

} // namespace automa
