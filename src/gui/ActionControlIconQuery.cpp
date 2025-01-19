#include "fornani/gui/ActionControlIconQuery.hpp"
#include "fornani/service/ServiceProvider.hpp"

static auto get_controller_lookup(EInputActionOrigin btn) -> int {
	if (btn >= EInputActionOrigin::k_EInputActionOrigin_PS4_X && btn <= EInputActionOrigin::k_EInputActionOrigin_PS4_Reserved10) { return 50; }
	if (btn >= EInputActionOrigin::k_EInputActionOrigin_XBoxOne_A && btn <= EInputActionOrigin::k_EInputActionOrigin_XBoxOne_Reserved10) { return 114; }
	if (btn >= EInputActionOrigin::k_EInputActionOrigin_XBox360_A && btn <= EInputActionOrigin::k_EInputActionOrigin_XBox360_Reserved10) { return 153; }
	if (btn >= EInputActionOrigin::k_EInputActionOrigin_Switch_A && btn <= EInputActionOrigin::k_EInputActionOrigin_Switch_Reserved10) { return 192; }
	if (btn >= EInputActionOrigin::k_EInputActionOrigin_PS5_X && btn <= EInputActionOrigin::k_EInputActionOrigin_PS5_Reserved20) { return 258; }
	return 0;
}

static auto get_icon_lookup(EInputActionOrigin btn) -> int {
	switch (get_controller_lookup(btn)) {
	case 50: return 0; break;
	case 114: return 8; break;
	case 153: return 8; break;
	case 192: return 10; break;
	case 258: return 4; break;
	default: return 0;
	}
}

sf::Vector2i get_key_coordinates(sf::Keyboard::Scancode key) {
	auto keyi = static_cast<int>(key);
	auto controller_section = 13; // keyboard button atlas lookup
	if (keyi >= static_cast<int>(sf::Keyboard::Key::A) && keyi <= static_cast<int>(sf::Keyboard::Key::Pause)) {
		return {keyi % atlas_width, controller_section + keyi / atlas_width};
	} else {
		return {controller_section, 3};
	}
}

sf::Vector2i get_controller_button_coordinates(EInputActionOrigin btn) {
	auto controller_index = get_controller_lookup(btn);
	auto buttoni = static_cast<int>(btn) - controller_index;
	auto controller_section = get_icon_lookup(btn);
	return {buttoni % atlas_width, controller_section + buttoni / atlas_width};
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

	sf::Sprite sprite{svc.assets.t_controller_button_icons};
	auto dimensions = sf::Vector2<int>{36, 36};
	sprite.setTextureRect(sf::IntRect{lookup * 36, dimensions});
	sprite.setOrigin(sf::Vector2<float>{0.f, 8.f});

	return sprite;
}

} // namespace gui
