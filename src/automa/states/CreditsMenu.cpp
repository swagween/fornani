
#include "CreditsMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

CreditsMenu::CreditsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {
	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);
}

void CreditsMenu::init(ServiceProvider& svc, std::string_view room) {}

void CreditsMenu::setTilesetTexture(ServiceProvider& svc, sf::Texture& t) {}

void CreditsMenu::handle_events(ServiceProvider& svc, sf::Event& event) {
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
	if (svc.controller_map.label_to_control.at("left").triggered()) {
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	if (svc.controller_map.label_to_control.at("right").triggered()) {}
	if (svc.controller_map.label_to_control.at("main_action").triggered()) {}
	if (svc.controller_map.label_to_control.at("secondary_action").triggered()) {
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	if (event.type == sf::Event::EventType::JoystickMoved) { svc.controller_map.reset_triggers(); }
}

void CreditsMenu::tick_update(ServiceProvider& svc) {
	for (auto& option : options) { option.update(svc, current_selection); }
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection).left_offset);
	right_dot.set_target_position(options.at(current_selection).right_offset);

	svc.soundboard.play_sounds(svc);
	svc.controller_map.reset_triggers();
}

void CreditsMenu::frame_update(ServiceProvider& svc) {}

void CreditsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {

	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
