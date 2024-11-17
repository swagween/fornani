#include "ActionControlIconQuery.hpp"

#include "../service/ServiceProvider.hpp"

sf::Vector2i get_key_coordinates(sf::Keyboard::Key key) {
	auto keyi = static_cast<int>(key);
	if (keyi >= static_cast<int>(sf::Keyboard::Key::A) && keyi <= static_cast<int>(sf::Keyboard::Key::Pause)) {
		return {keyi % 14, 3 + keyi / 14};
	} else {
		return {10, 3};
	}
}

sf::Vector2i get_controller_button_coordinates(EInputActionOrigin btn) {
	switch (btn) {
	case k_EInputActionOrigin_XBoxOne_A: return {0, 0};
	case k_EInputActionOrigin_XBoxOne_B: return {1, 0};
	case k_EInputActionOrigin_XBoxOne_X: return {2, 0};
	case k_EInputActionOrigin_XBoxOne_Y: return {3, 0};
	case k_EInputActionOrigin_XBoxOne_LeftBumper: return {0, 1};
	case k_EInputActionOrigin_XBoxOne_RightBumper: return {1, 1};
	case k_EInputActionOrigin_XBoxOne_Menu: return {12, 0}; // Start
	case k_EInputActionOrigin_XBoxOne_View: return {13, 0}; // Back
	case k_EInputActionOrigin_XBoxOne_LeftTrigger_Pull: return {2, 1};
	case k_EInputActionOrigin_XBoxOne_LeftTrigger_Click: return {2, 1};
	case k_EInputActionOrigin_XBoxOne_RightTrigger_Pull: return {3, 1};
	case k_EInputActionOrigin_XBoxOne_RightTrigger_Click: return {3, 1};
	case k_EInputActionOrigin_XBoxOne_LeftStick_Move: return {0, 2};
	case k_EInputActionOrigin_XBoxOne_LeftStick_Click: return {12, 1};
	case k_EInputActionOrigin_XBoxOne_DPad_North: return {6, 1};
	case k_EInputActionOrigin_XBoxOne_DPad_South: return {8, 1};
	case k_EInputActionOrigin_XBoxOne_DPad_West: return {5, 1};
	case k_EInputActionOrigin_XBoxOne_DPad_East:
		return {7, 1};
		/*
		case k_EInputActionOrigin_XBoxOne_LeftStick_DPadNorth: return {};
		case k_EInputActionOrigin_XBoxOne_LeftStick_DPadSouth: return {};
		case k_EInputActionOrigin_XBoxOne_LeftStick_DPadWest: return {};
		case k_EInputActionOrigin_XBoxOne_LeftStick_DPadEast: return {};
		case k_EInputActionOrigin_XBoxOne_RightStick_Move: return {};
		case k_EInputActionOrigin_XBoxOne_RightStick_Click: return {};
		case k_EInputActionOrigin_XBoxOne_RightStick_DPadNorth: return {};
		case k_EInputActionOrigin_XBoxOne_RightStick_DPadSouth: return {};
		case k_EInputActionOrigin_XBoxOne_RightStick_DPadWest: return {};
		case k_EInputActionOrigin_XBoxOne_RightStick_DPadEast: return {};
		case k_EInputActionOrigin_XBoxOne_DPad_Move: return {};
		case k_EInputActionOrigin_XBoxOne_LeftGrip_Lower: return {};
		case k_EInputActionOrigin_XBoxOne_LeftGrip_Upper: return {};
		case k_EInputActionOrigin_XBoxOne_RightGrip_Lower: return {};
		case k_EInputActionOrigin_XBoxOne_RightGrip_Upper: return {};
		case k_EInputActionOrigin_XBoxOne_Share: return {}; // Xbox Series X controllers only
		case k_EInputActionOrigin_XBoxOne_Reserved6: return {};
		case k_EInputActionOrigin_XBoxOne_Reserved7: return {};
		case k_EInputActionOrigin_XBoxOne_Reserved8: return {};
		case k_EInputActionOrigin_XBoxOne_Reserved9: return {};
		case k_EInputActionOrigin_XBoxOne_Reserved10: return {};
		*/

	default: return {13, 2};
	}
}

namespace gui {

auto get_action_control_icon(automa::ServiceProvider& svc, config::DigitalAction action) -> sf::Sprite {
	auto source = svc.controller_map.digital_action_source(action);

	sf::Vector2i lookup{};
	if (source.controller_origin == k_EInputActionOrigin_None) {
		lookup = get_key_coordinates(source.key);
	} else {
		lookup = get_controller_button_coordinates(source.controller_origin);
	}

	sf::Sprite sprite;
	sprite.setTexture(svc.assets.t_controller_button_icons);
	auto dimensions = sf::Vector2<int>{36, 36};
	sprite.setTextureRect(sf::IntRect{lookup * 36, dimensions});
	sprite.setOrigin(sf::Vector2<float>{0.f, 8.f});

	return sprite;
}

} // namespace gui
