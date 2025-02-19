
#include "fornani/automa/states/ControlsMenu.hpp"
#include <algorithm>
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

constexpr std::array<std::string_view, 4> tabs = {"controls_platformer", "controls_inventory", "controls_map", "controls_menu"};
constexpr std::array<std::string_view, 4> tab_id_prefixes = {"platformer_", "inventory_", "map_", "menu_"};

ControlsMenu::ControlsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number) : GameState(svc, player, scene, room_number), instruction(svc.text.fonts.title) {
	change_scene(svc, "controls_platformer");
	refresh_controls(svc);
	instruction.setLineSpacing(1.5f);
	instruction.setLetterSpacing(title_letter_spacing);
	instruction.setCharacterSize(options.at(current_selection.get()).label.getCharacterSize());
	instruction.setPosition({svc.constants.screen_dimensions.x * 0.5f - instruction.getLocalBounds().getCenter().x, svc.constants.screen_dimensions.y - 120.f});
	instruction.setFillColor(svc.styles.colors.dark_grey);

	left_dot.set_position(options.at(current_selection.get()).left_offset);
	right_dot.set_position(options.at(current_selection.get()).right_offset);
	loading.start(2);

	debug.setFillColor(sf::Color::Transparent);
	debug.setSize({2.f, (float)svc.constants.screen_dimensions.y});
	debug.setOutlineColor(svc.styles.colors.blue);
	debug.setOutlineThickness(-1);
}

void ControlsMenu::tick_update(ServiceProvider& svc) {
	binding_mode ? flags.reset(GameStateFlags::ready) : flags.set(GameStateFlags::ready);
	svc.controller_map.set_action_set(config::ActionSet::Menu);

	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered && !binding_mode) {
		current_selection.modulate(-1);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered && !binding_mode) {
		current_selection.modulate(1);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_left).triggered && option_is_selected && current_selection.get() == 0) {
		auto current_tab = std::distance(tabs.begin(), std::find(tabs.begin(), tabs.end(), scene));
		if (current_tab == 0) { current_tab = 4; }
		auto tab_to_switch_to = current_tab - 1;
		change_scene(svc, tabs[tab_to_switch_to]);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_right).triggered && option_is_selected && current_selection.get() == 0) {
		auto current_tab = std::distance(tabs.begin(), std::find(tabs.begin(), tabs.end(), scene));
		auto tab_to_switch_to = (current_tab + 1) % 4;
		change_scene(svc, tabs[tab_to_switch_to]);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) {
		if (binding_mode) {
			binding_mode = false;
			option_is_selected = false;
			auto& control = control_list.at(current_selection.get());
			restore_defaults(svc);
			// TODO: set new control here. this is a temporary fix for the hard lock
		} else {
			svc.state_controller.submenu = MenuType::options;
			svc.state_controller.actions.set(Actions::exit_submenu);
		}
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	auto pressed_select = svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered;
	if (pressed_select) {
		// Gamepad settings should be second to last option
		if (current_selection.get() == 0) {
			option_is_selected = !option_is_selected;
		} else if (current_selection.get() == options.size() - 2) {
			svc.controller_map.open_bindings_overlay();
		}
		// Reset to default should be last option
		else if (current_selection.get() == options.size() - 1) {
			restore_defaults(svc);
		} else if (!binding_mode) {
			option_is_selected = !option_is_selected;
			binding_mode = true;
			auto& control = control_list.at(current_selection.get());
			control.setString("Press a key");
			control.setOrigin({control.getLocalBounds().size.x, control.getLocalBounds().getCenter().y});
		}
	}

	for (auto& option : options) {
		option.flagged = current_selection.get() == option.index && option_is_selected;
		option.update(svc, current_selection.get());
	}

	loading.update();
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection.get()).left_offset);
	right_dot.set_target_position(options.at(current_selection.get()).right_offset);

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
	size_t ctr{0};
	for (auto& option : options) {
		if (ctr > 0 && ctr < options.size() - 2) {
			auto current_tab = std::distance(tabs.begin(), std::find(tabs.begin(), tabs.end(), scene));
			auto id = std::string(tab_id_prefixes.at(current_tab)) + static_cast<std::string>(option.label.getString());
			auto action = svc.controller_map.get_action_by_identifier(id.data());

			auto& control = control_list.at(ctr);
			control.setString(std::string(svc.controller_map.key_to_string(svc.controller_map.get_primary_keyboard_binding(action))));
			control.setOrigin({control.getLocalBounds().size.x, control.getLocalBounds().getCenter().y});
			control.setPosition({svc.constants.screen_dimensions.x * 0.5f + center_offset, option.position.y});
			control.setCharacterSize(16);
			control.setLetterSpacing(title_letter_spacing);
			control.setFillColor(svc.styles.colors.dark_grey);
			control.setOrigin(control.getLocalBounds().getCenter());
		}
		++ctr;
	}
	// FIXME User may get stuck if their menu bindings are wrong, because they won't be able to reset them
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
	if (!options.empty()) { current_selection = util::Circuit(static_cast<int>(options.size())); }
	top_buffer = svc.data.menu["config"][scene]["top_buffer"].as<float>();
	int ctr{};
	for (auto& option : options) {
		if (ctr == 0 || ctr >= options.size() - 2) {
			// Show tab selector, gamepad settings & reset to default in middle of screen
			option.position.x = svc.constants.screen_dimensions.x * 0.5f;
		} else {
			option.position.x = svc.constants.screen_dimensions.x * 0.5f - center_offset + option.label.getLocalBounds().getCenter().x;
		}
		// FIXME Spacing is broken in other menus. getLocalBounds().height is returning 0 because the font isn't set when the function is called
		// 	     To make up for it we don't add the getLocalBounds().height factor here, but keep it in mind when it is fixed!
		option.position.y = top_buffer + ctr * (spacing);
		option.index = ctr;
		option.update(svc, current_selection.get());

		control_list.push_back(sf::Text(svc.text.fonts.title));
		++ctr;
	}
	refresh_controls(svc);
}

} // namespace fornani::automa
