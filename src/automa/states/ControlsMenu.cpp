
#include "ControlsMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

ControlsMenu::ControlsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {
	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);

	int ctr{0};
	for (auto& option : options) {
		option.position.x = svc.constants.screen_dimensions.x * 0.5 - center_offset;
		option.label.setLetterSpacing(0.6f);
		control_list.push_back(sf::Text());
		control_list.back().setString(svc.controller_map.tag_to_label.at(svc.controller_map.tags.at(ctr)).data());
		control_list.back().setFont(font);
		control_list.back().setOrigin(control_list.back().getLocalBounds().width * 0.5f, 0.f);
		control_list.back().setCharacterSize(option.label.getCharacterSize());
		control_list.back().setPosition(svc.constants.screen_dimensions.x * 0.5 + center_offset, option.position.y);
		++ctr;
	}
}

void ControlsMenu::init(ServiceProvider& svc, std::string_view room) {}

void ControlsMenu::setTilesetTexture(ServiceProvider& svc, sf::Texture& t) {}

void ControlsMenu::handle_events(ServiceProvider& svc, sf::Event& event) {
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

void ControlsMenu::tick_update(ServiceProvider& svc) {
	int ctr{0};
	for (auto& option : options) {
		option.update(svc, current_selection);
		control_list.at(ctr).setFillColor(option.label.getFillColor());
		++ctr;
	}
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection).left_offset);
	right_dot.set_target_position(options.at(current_selection).right_offset);

	svc.soundboard.play_sounds(svc);
	svc.controller_map.reset_triggers();
}

void ControlsMenu::frame_update(ServiceProvider& svc) {}

void ControlsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {

	for (auto& option : options) { win.draw(option.label); }
	for (auto& control : control_list) { win.draw(control); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
