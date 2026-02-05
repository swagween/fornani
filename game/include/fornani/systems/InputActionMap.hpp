#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <string_view>

namespace fornani::input {

enum class AnalogAction : int { move, pan, map_pan, END };

enum class MoveDirection { left, right, up, down };

enum class DigitalAction : int {
	jump,
	shoot,
	sprint,
	slide,
	dash,
	inspect,
	tab_left,
	tab_right,
	pause,
	inventory,
	menu_up,
	menu_down,
	menu_left,
	menu_right,
	menu_tab_left,
	menu_tab_right,
	menu_select,
	menu_back,
	menu_confirm,
	menu_close,
	END
};

inline std::unordered_map<std::string_view, DigitalAction> const string_to_action{{"jump", DigitalAction::jump},
																				  {"shoot", DigitalAction::shoot},
																				  {"sprint", DigitalAction::sprint},
																				  {"slide", DigitalAction::slide},
																				  {"dash", DigitalAction::dash},
																				  {"inspect", DigitalAction::inspect},
																				  {"tab_left", DigitalAction::tab_left},
																				  {"tab_right", DigitalAction::tab_right},
																				  {"inventory", DigitalAction::inventory},
																				  {"pause", DigitalAction::pause},
																				  {"menu_left", DigitalAction::menu_left},
																				  {"menu_right", DigitalAction::menu_right},
																				  {"menu_up", DigitalAction::menu_up},
																				  {"menu_down", DigitalAction::menu_down},
																				  {"menu_select", DigitalAction::menu_select},
																				  {"menu_back", DigitalAction::menu_back},
																				  {"menu_tab_left", DigitalAction::menu_tab_left},
																				  {"menu_tab_right", DigitalAction::menu_tab_right},
																				  {"menu_confirm", DigitalAction::menu_confirm},
																				  {"menu_close", DigitalAction::menu_close}};

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

inline std::unordered_map<sf::Keyboard::Scancode, std::string_view> const scancode_to_string = [] {
	std::unordered_map<sf::Keyboard::Scancode, std::string_view> map;
	for (auto const& [str, code] : string_to_scancode) { map.emplace(code, str); }
	return map;
}();

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

inline std::string_view string_from_scancode(sf::Keyboard::Scancode scancode) {
	auto it = scancode_to_string.find(scancode);
	if (it != scancode_to_string.end()) return it->second;

	return "Unknown";
}

} // namespace fornani::input
