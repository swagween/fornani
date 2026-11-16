
#include "fornani/automa/states/ControlsMenu.hpp"
#include <fornani/systems/InputActionMap.hpp>
#include "fornani/graphics/Colors.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Constants.hpp"

namespace fornani::automa {

constexpr std::array<std::string_view, 2> tabs = {"controls_platformer", "controls_menu"};
constexpr std::array<std::string_view, 2> tab_id_prefixes = {"", "menu_"};

constexpr auto number_of_extra_control_options_v = 3;

ControlsMenu::ControlsMenu(ServiceProvider& svc, player::Player& player, AppContext& ctx) : MenuState(svc, player, ctx, "controls_platformer"), m_current_tab{tabs.size()}, m_scene{"controls_platformer"} {
	m_parent_menu = MenuType::options;
	spacing = 20.f;
	change_scene(svc, m_scene);
	loading.start(2);

	debug.setFillColor(sf::Color::Transparent);
	debug.setSize({2.f, svc.window->f_screen_dimensions().y});
	debug.setOutlineColor(colors::blue);
	debug.setOutlineThickness(-1);

	refresh_controls(svc);
}

void ControlsMenu::on_exit() { p_app_context->settings.save_user_controls(*p_services); }

void ControlsMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	m_input_authorized = !binding_mode;
	MenuState::tick_update(svc, engine);
	binding_mode ? flags.reset(GameStateFlags::ready) : flags.set(GameStateFlags::ready);

	// reset gamepad settings color
	options.at(options.size() - number_of_extra_control_options_v).selectable = svc.input_system.is_gamepad();

	for (auto& option : options) {
		option.flagged = current_selection.get() == option.index && option_is_selected;
		option.update(current_selection.get());
	}

	if (m_helptext) { m_helptext->update(); }

	static auto action_to_bind = std::optional<input::DigitalAction>{};
	static auto last_key_pressed = std::optional<sf::Keyboard::Scancode>{};

	if (binding_mode) {
		if (!svc.input_system.consume_flag(input::InputSystemFlags::key_was_pressed)) { return; }
		if (svc.input_system.digital(input::DigitalAction::menu_select).held && !svc.input_system.digital(input::DigitalAction::menu_select).triggered) { return; }
		if (svc.input_system.digital(input::DigitalAction::menu_select).released) {
			last_key_pressed.reset();
			return;
		}
		if (svc.input_system.keys_pressed.empty()) { return; }
		last_key_pressed = svc.input_system.get_last_key_pressed();
		if (svc.input_system.digital(input::DigitalAction::menu_confirm).triggered) {
			if (svc.input_system.has_forbidden_duplicate_binding()) {
				svc.soundboard.flags.menu.set(audio::Menu::error);
			} else {
				exit_binding_mode();
				svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
				if (action_to_bind) {
					bool primary = true; // TODO: maybe implement secondary bindings later
					auto action_name = options.at(current_selection.get()).label.getString().toAnsiString();
					auto binding = primary ? svc.input_system.get_primary_keyboard_binding(*action_to_bind) : svc.input_system.get_secondary_keyboard_binding(*action_to_bind);
					p_app_context->settings.serialize_control_binding(action_name, input::string_from_scancode(binding), primary);
				}
				refresh_controls(svc);
			}
		}
		if (svc.input_system.get_last_key_pressed() == sf::Keyboard::Scancode::Escape) {
			exit_binding_mode();
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
			refresh_controls(svc);
		}
		svc.soundboard.flags.menu.set(audio::Menu::select);
		if (binding_mode && svc.input_system.was_keyboard_input_detected() && action_to_bind && last_key_pressed) { svc.input_system.set_primary_keyboard_binding(action_to_bind.value(), svc.input_system.get_last_key_pressed()); }

		refresh_controls(svc);

		return;
	}

	auto this_selection = current_selection.get();
	if (svc.input_system.menu_move(input::MoveDirection::up)) {
		while (!options.at(current_selection.get()).selectable && current_selection.get() != this_selection) { current_selection.modulate(-1); }
		option_is_selected = false;
	}
	if (svc.input_system.menu_move(input::MoveDirection::down)) {
		while (!options.at(current_selection.get()).selectable && current_selection.get() != this_selection) { current_selection.modulate(1); }
		option_is_selected = false;
	}
	if ((svc.input_system.menu_move(input::MoveDirection::left) && current_selection.get() == 0) || svc.input_system.digital(input::DigitalAction::menu_tab_left).triggered) {
		m_current_tab.modulate(-1);
		change_scene(svc, tabs[m_current_tab.get()]);
	}
	if ((svc.input_system.menu_move(input::MoveDirection::right) && current_selection.get() == 0) || svc.input_system.digital(input::DigitalAction::menu_tab_right).triggered) {
		m_current_tab.modulate(1);
		change_scene(svc, tabs[m_current_tab.get()]);
	}
	if (was_selected(svc.input_system)) {
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		// Gamepad settings should be second to last option
		if (current_selection.get() == 0) {
			option_is_selected = !option_is_selected;
		} else if (current_selection.get() == options.size() - 3) {
			svc.input_system.open_bindings_overlay();
		} else if (current_selection.get() == options.size() - 2) { // WASD defaults
			restore_defaults(svc, true);
			refresh_controls(svc);
		} else if (current_selection.get() == options.size() - 1) { // defaults
			restore_defaults(svc);
			refresh_controls(svc);
		} else {
			option_is_selected = !option_is_selected;
			enter_binding_mode();
			refresh_controls(svc);
			action_to_bind.reset();
			auto& control = control_list.at(current_selection.get());
			control.setString(svc.data.gui_text["controls"]["binding_prompt"].as_string());
			auto id = std::string(tab_id_prefixes.at(m_current_tab.get())) + std::string(options.at(current_selection.get()).label.getString());
			action_to_bind = input::action_from_string(id);
			control.setOrigin({control.getLocalBounds().size.x, control.getLocalBounds().getCenter().y});
		}
	}

	loading.update();
}

void ControlsMenu::frame_update(ServiceProvider& svc) {}

void ControlsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	MenuState::render(svc, win);
	if (loading.is_complete()) {
		for (auto& option : options) { win.draw(option.label); }
		for (auto& control : control_list) { win.draw(control); }
		if (m_helptext) { m_helptext->render(win); }
	}
}

void ControlsMenu::refresh_controls(ServiceProvider& svc) {
	std::size_t ctr{0};
	for (auto [i, option] : std::views::enumerate(options)) {
		option.update(current_selection.get());
		if (ctr > 0 && ctr < options.size() - number_of_extra_control_options_v) {
			auto current_tab = std::distance(tabs.begin(), std::find(tabs.begin(), tabs.end(), m_scene));
			auto id = current_tab > 0 ? std::string{tab_id_prefixes.at(current_tab)} + std::string{option.label.getString()} : std::string{option.label.getString()};
			auto action = input::action_from_string(id);

			auto& control = control_list.at(ctr);
			control.setString(std::string(input::string_from_scancode(svc.input_system.get_primary_keyboard_binding(action))));
			control.setOrigin({control.getLocalBounds().size.x, control.getLocalBounds().getCenter().y});
			control.setPosition({svc.window->i_screen_dimensions().x * 0.5f + center_offset, option.position.y});
			control.setCharacterSize(svc.text.fonts.title.glyph_size);
			control.setLetterSpacing(1.f);
			control.setFillColor(option.label.getFillColor());
			control.setOrigin(control.getLocalBounds().getCenter());
			option.selectable = !svc.input_system.is_gamepad();
		}
		option.selectable = binding_mode ? i == current_selection.get() : true;
		++ctr;
	}
}

void ControlsMenu::restore_defaults(ServiceProvider& svc, bool wasd) {
	p_app_context->settings.reset_user_controls(svc, wasd);
	p_app_context->settings.save_user_controls(svc);
	p_app_context->settings.set_user_controls(svc.input_system);
}

void ControlsMenu::change_scene(ServiceProvider& svc, std::string_view to_change_to) {
	m_scene = to_change_to;
	options.clear();
	control_list.clear();
	auto const& in_data = svc.data.menu["options"];
	for (auto& entry : in_data[to_change_to].as_array()) { options.push_back(Option(svc, p_app_context->settings.get_theme(), entry.as_string())); }
	if (!options.empty()) { current_selection = util::Circuit(static_cast<int>(options.size())); }
	top_buffer = svc.data.menu["config"][to_change_to]["top_buffer"].as<float>();
	int ctr{};
	for (auto& option : options) {
		if (ctr == 0 || ctr >= options.size() - number_of_extra_control_options_v) {
			// Show tab selector, gamepad settings & reset to default in middle of screen
			option.position.x = svc.window->i_screen_dimensions().x * 0.5f;
		} else {
			option.position.x = svc.window->i_screen_dimensions().x * 0.5f - center_offset + option.label.getLocalBounds().getCenter().x;
		}
		option.position.y = top_buffer + ctr * (spacing);
		option.index = ctr;
		option.update(current_selection.get());

		control_list.push_back(sf::Text(svc.text.fonts.title.font));
		++ctr;
	}
	refresh_controls(svc);
}

void ControlsMenu::enter_binding_mode() {
	m_helptext.emplace(*p_services, p_services->data.gui_text["controls"]["confirm_begin"].as_string(), fornani::input::DigitalAction::menu_confirm, p_services->data.gui_text["controls"]["confirm_end"].as_string(), 1);
	binding_mode = true;
}

void ControlsMenu::exit_binding_mode() {
	m_helptext.reset();
	binding_mode = false;
	option_is_selected = false;
}

} // namespace fornani::automa
