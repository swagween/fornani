#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <string_view>

namespace fornani::input {

enum class DigitalAction : int {
	// Platformer
	platformer_left,
	platformer_right,
	platformer_up,
	platformer_down,
	platformer_jump,
	platformer_shoot,
	platformer_sprint,
	platformer_slide,
	platformer_dash,
	platformer_inspect,
	platformer_arms_switch_left,
	platformer_arms_switch_right,
	platformer_open_inventory,
	platformer_toggle_pause,

	// Inventory
	inventory_close,

	// Menu
	menu_left,
	menu_right,
	menu_up,
	menu_down,
	menu_select,
	menu_cancel,
	menu_tab_left,
	menu_tab_right,
	menu_confirm,

	END
};

inline std::unordered_map<std::string_view, DigitalAction> const string_to_action{{"platformer_left", DigitalAction::platformer_left},
																				  {"platformer_right", DigitalAction::platformer_right},
																				  {"platformer_up", DigitalAction::platformer_up},
																				  {"platformer_down", DigitalAction::platformer_down},
																				  {"platformer_jump", DigitalAction::platformer_jump},
																				  {"platformer_shoot", DigitalAction::platformer_shoot},
																				  {"platformer_sprint", DigitalAction::platformer_sprint},
																				  {"platformer_slide", DigitalAction::platformer_slide},
																				  {"platformer_dash", DigitalAction::platformer_dash},
																				  {"platformer_inspect", DigitalAction::platformer_inspect},
																				  {"platformer_arms_switch_left", DigitalAction::platformer_arms_switch_left},
																				  {"platformer_arms_switch_right", DigitalAction::platformer_arms_switch_right},
																				  {"platformer_open_inventory", DigitalAction::platformer_open_inventory},
																				  {"platformer_toggle_pause", DigitalAction::platformer_toggle_pause},
																				  {"inventory_close", DigitalAction::inventory_close},
																				  {"menu_left", DigitalAction::menu_left},
																				  {"menu_right", DigitalAction::menu_right},
																				  {"menu_up", DigitalAction::menu_up},
																				  {"menu_down", DigitalAction::menu_down},
																				  {"menu_select", DigitalAction::menu_select},
																				  {"menu_cancel", DigitalAction::menu_cancel},
																				  {"menu_tab_left", DigitalAction::menu_tab_left},
																				  {"menu_tab_right", DigitalAction::menu_tab_right},
																				  {"menu_confirm", DigitalAction::menu_confirm}};

inline std::unordered_map<std::string_view, sf::Keyboard::Scancode> const string_to_scancode{{"A", sf::Keyboard::Scancode::A},			 {"B", sf::Keyboard::Scancode::B},
																							 {"C", sf::Keyboard::Scancode::C},			 {"D", sf::Keyboard::Scancode::D},
																							 {"E", sf::Keyboard::Scancode::E},			 {"F", sf::Keyboard::Scancode::F},
																							 {"G", sf::Keyboard::Scancode::G},			 {"H", sf::Keyboard::Scancode::H},
																							 {"I", sf::Keyboard::Scancode::I},			 {"J", sf::Keyboard::Scancode::J},
																							 {"K", sf::Keyboard::Scancode::K},			 {"L", sf::Keyboard::Scancode::L},
																							 {"M", sf::Keyboard::Scancode::M},			 {"N", sf::Keyboard::Scancode::N},
																							 {"O", sf::Keyboard::Scancode::O},			 {"P", sf::Keyboard::Scancode::P},
																							 {"Q", sf::Keyboard::Scancode::Q},			 {"R", sf::Keyboard::Scancode::R},
																							 {"S", sf::Keyboard::Scancode::S},			 {"T", sf::Keyboard::Scancode::T},
																							 {"U", sf::Keyboard::Scancode::U},			 {"V", sf::Keyboard::Scancode::V},
																							 {"W", sf::Keyboard::Scancode::W},			 {"X", sf::Keyboard::Scancode::X},
																							 {"Y", sf::Keyboard::Scancode::Y},			 {"Z", sf::Keyboard::Scancode::Z},
																							 {"LShift", sf::Keyboard::Scancode::LShift}, {"RShift", sf::Keyboard::Scancode::RShift},
																							 {"Left", sf::Keyboard::Scancode::Left},	 {"Right", sf::Keyboard::Scancode::Right},
																							 {"Up", sf::Keyboard::Scancode::Up},		 {"Down", sf::Keyboard::Scancode::Down},
																							 {"Period", sf::Keyboard::Scancode::Period}, {"1", sf::Keyboard::Scancode::Num1},
																							 {"2", sf::Keyboard::Scancode::Num2},		 {"3", sf::Keyboard::Scancode::Num3},
																							 {"Space", sf::Keyboard::Scancode::Space},	 {"LControl", sf::Keyboard::Scancode::LControl},
																							 {"Esc", sf::Keyboard::Scancode::Escape},	 {"Enter", sf::Keyboard::Scancode::Enter}};

// --- helpers ---
inline DigitalAction action_from_string(std::string_view str) {
	auto it = string_to_action.find(str);
	if (it != string_to_action.end()) { return it->second; }
	return DigitalAction::END;
}

inline sf::Keyboard::Scancode scancode_from_string(std::string_view str) {
	auto it = string_to_scancode.find(str);
	if (it != string_to_scancode.end()) return it->second;
	return sf::Keyboard::Scancode::Unknown;
}

} // namespace fornani::input
