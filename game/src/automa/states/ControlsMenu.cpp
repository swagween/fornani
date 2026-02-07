
#include "fornani/automa/states/ControlsMenu.hpp"
#include <fornani/systems/InputActionMap.hpp>
#include "fornani/graphics/Colors.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Constants.hpp"

namespace fornani::automa {

constexpr std::array<std::string_view, 2> tabs = {"controls_platformer", "controls_menu"};
constexpr std::array<std::string_view, 2> tab_id_prefixes = {"platformer_", "menu_"};

ControlsMenu::ControlsMenu(ServiceProvider& svc, player::Player& player) : MenuState(svc, player, "controls_platformer"), instruction(svc.text.fonts.title), m_current_tab{tabs.size()}, m_scene{"controls_platformer"} {
	m_parent_menu = MenuType::options;
	change_scene(svc, m_scene);
	instruction.setLineSpacing(1.5f);
	instruction.setLetterSpacing(1.f);
	instruction.setCharacterSize(options.at(current_selection.get()).label.getCharacterSize());
	instruction.setPosition({svc.window->i_screen_dimensions().x * 0.5f - instruction.getLocalBounds().getCenter().x, svc.window->i_screen_dimensions().y - 120.f});
	instruction.setFillColor(colors::dark_grey);
	loading.start(2);

	debug.setFillColor(sf::Color::Transparent);
	debug.setSize({2.f, svc.window->f_screen_dimensions().y});
	debug.setOutlineColor(colors::blue);
	debug.setOutlineThickness(-1);
}

void ControlsMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	m_input_authorized = !binding_mode;
	MenuState::tick_update(svc, engine);
	binding_mode ? flags.reset(GameStateFlags::ready) : flags.set(GameStateFlags::ready);

	// reset gamepad settings color
	options.at(options.size() - 2).selectable = svc.input_system.is_gamepad_connected();

	static bool entered{};
	if (binding_mode) {
		if (svc.input_system.digital(input::DigitalAction::menu_confirm).triggered) {
			if (svc.input_system.has_forbidden_duplicate_binding()) {
				svc.soundboard.flags.menu.set(audio::Menu::error);
			} else {
				binding_mode = false;
				option_is_selected = false;
				svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
			}
		}
		auto id = std::string(tab_id_prefixes.at(m_current_tab.get())) + std::string(options.at(current_selection.get()).label.getString());
		auto action = input::action_from_string(id.data());
		if (binding_mode && svc.input_system.was_keyboard_input_detected() && entered) { svc.input_system.set_primary_keyboard_binding(action, svc.input_system.get_last_key_pressed()); }
		entered = true;
	}

	auto this_selection = current_selection.get();
	if (svc.input_system.menu_move(input::MoveDirection::up) && !binding_mode) {
		while (!options.at(current_selection.get()).selectable && current_selection.get() != this_selection) { current_selection.modulate(-1); }
		option_is_selected = false;
	}
	if (svc.input_system.menu_move(input::MoveDirection::down) && !binding_mode) {
		while (!options.at(current_selection.get()).selectable && current_selection.get() != this_selection) { current_selection.modulate(1); }
		option_is_selected = false;
	}
	if (svc.input_system.menu_move(input::MoveDirection::left) && option_is_selected && current_selection.get() == 0) {
		m_current_tab.modulate(-1);
		change_scene(svc, tabs[m_current_tab.get()]);
	}
	if (svc.input_system.menu_move(input::MoveDirection::right) && option_is_selected && current_selection.get() == 0) {
		m_current_tab.modulate(1);
		change_scene(svc, tabs[m_current_tab.get()]);
	}
	if (svc.input_system.digital(input::DigitalAction::menu_select).triggered) {
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		// Gamepad settings should be second to last option
		if (current_selection.get() == 0) {
			option_is_selected = !option_is_selected;
		} else if (current_selection.get() == options.size() - 2) {
			svc.input_system.open_bindings_overlay();
		}
		// Reset to default should be last option
		else if (current_selection.get() == options.size() - 1) {
			restore_defaults(svc);
		} else if (!binding_mode) {
			option_is_selected = !option_is_selected;
			binding_mode = true;
			entered = false;
			auto& control = control_list.at(current_selection.get());
			control.setString("Press a key (Enter to confirm)");
			control.setOrigin({control.getLocalBounds().size.x, control.getLocalBounds().getCenter().y});
		}
	}

	for (auto& option : options) {
		option.flagged = current_selection.get() == option.index && option_is_selected;
		option.update(svc, current_selection.get());
	}

	refresh_controls(svc);

	loading.update();
}

void ControlsMenu::frame_update(ServiceProvider& svc) {}

void ControlsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	MenuState::render(svc, win);
	if (loading.is_complete()) {
		for (auto& option : options) { win.draw(option.label); }
		for (auto& control : control_list) { win.draw(control); }
		win.draw(instruction);
	}
}

void ControlsMenu::refresh_controls(ServiceProvider& svc) {
	std::size_t ctr{0};
	for (auto& option : options) {
		option.update(svc, current_selection.get());
		if (ctr > 0 && ctr < options.size() - 3) {
			auto current_tab = std::distance(tabs.begin(), std::find(tabs.begin(), tabs.end(), m_scene));
			auto id = current_tab > 0 ? std::string{tab_id_prefixes.at(current_tab)} + std::string{option.label.getString()} : std::string{option.label.getString()};
			auto action = input::action_from_string(id.data());

			auto& control = control_list.at(ctr);
			control.setString(std::string(input::string_from_scancode(svc.input_system.get_primary_keyboard_binding(action))));
			control.setOrigin({control.getLocalBounds().size.x, control.getLocalBounds().getCenter().y});
			control.setPosition({svc.window->i_screen_dimensions().x * 0.5f + center_offset, option.position.y});
			control.setCharacterSize(16);
			control.setLetterSpacing(1.f);
			control.setFillColor(option.label.getFillColor());
			control.setOrigin(control.getLocalBounds().getCenter());
			option.selectable = !svc.input_system.is_gamepad_connected() && svc.input_system.is_gamepad_input_enabled();
		}
		++ctr;
	}
}

void ControlsMenu::restore_defaults(ServiceProvider& svc) {
	svc.data.reset_controls();
	svc.data.save_controls(svc.input_system);
	svc.data.load_controls(svc.input_system);
}

void ControlsMenu::change_scene(ServiceProvider& svc, std::string_view to_change_to) {

	m_scene = to_change_to;
	options.clear();
	control_list.clear();
	auto const& in_data = svc.data.menu["options"];
	for (auto& entry : in_data[to_change_to].as_array()) { options.push_back(Option(svc, p_theme, entry.as_string())); }
	if (!options.empty()) { current_selection = util::Circuit(static_cast<int>(options.size())); }
	top_buffer = svc.data.menu["config"][to_change_to]["top_buffer"].as<float>();
	int ctr{};
	for (auto& option : options) {
		if (ctr == 0 || ctr >= options.size() - 2) {
			// Show tab selector, gamepad settings & reset to default in middle of screen
			option.position.x = svc.window->i_screen_dimensions().x * 0.5f;
		} else {
			option.position.x = svc.window->i_screen_dimensions().x * 0.5f - center_offset + option.label.getLocalBounds().getCenter().x;
		}
		// FIXME Spacing is broken in other menus. getLocalBounds().height is returning 0 because the font isn't set when the function is called
		// 	     To make up for it we don't add the getLocalBounds().height factor here, but keep it in mind when it is fixed!
		option.position.y = top_buffer + ctr * (spacing);
		option.index = ctr;
		option.update(svc, current_selection.get());

		control_list.push_back(sf::Text(svc.text.fonts.title));
		++ctr;
	}
}

} // namespace fornani::automa
