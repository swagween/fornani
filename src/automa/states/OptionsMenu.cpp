
#include "OptionsMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

Option::Option(ServiceProvider& svc, std::string_view lbl) {
	label.setString(lbl.data());
	label.setCharacterSize(24);
	label.setFillColor(svc.styles.colors.ui_white);
	label.setOrigin(label.getLocalBounds().width * 0.5f, label.getLocalBounds().height * 0.5f);
}

void Option::update(ServiceProvider& svc, int& selection) {
	label.setPosition(position);
	left_offset = position - sf::Vector2<float>{label.getLocalBounds().width * 0.5f, dot_offset.y};
	right_offset = position + sf::Vector2<float>{label.getLocalBounds().width * 0.5f, dot_offset.y};
	selection == index ? label.setFillColor(svc.styles.colors.ui_white) : label.setFillColor(svc.styles.colors.dark_grey);
}

OptionsMenu::OptionsMenu(ServiceProvider& svc, player::Player& player, int id) : GameState(svc, player, id) {
	options.push_back(Option(svc, "Controls"));
	options.push_back(Option(svc, "Credits"));
	font.loadFromFile(svc.text.font);
	font.setSmooth(false);
	int ctr{};
	for (auto& option : options) {
		option.position = {svc.constants.screen_dimensions.x * 0.5f - option.label.getLocalBounds().width * 0.5f, ctr * (option.label.getLocalBounds().height + spacing)};
		option.label.setFont(font);
		option.index = ctr;
		++ctr;
	}
}

void OptionsMenu::init(ServiceProvider& svc, std::string_view room) {}

void OptionsMenu::setTilesetTexture(ServiceProvider& svc, sf::Texture& t) {}

void OptionsMenu::handle_events(ServiceProvider& svc, sf::Event& event) {
	svc.controller_map.handle_mouse_events(event);
	svc.controller_map.handle_joystick_events(event);
	if (event.type == sf::Event::EventType::KeyPressed) { svc.controller_map.handle_press(event.key.code); }
	if (event.type == sf::Event::EventType::KeyReleased) { svc.controller_map.handle_release(event.key.code); }

	if (svc.controller_map.label_to_control.at("down").triggered()) {}
	if (svc.controller_map.label_to_control.at("up").triggered()) {}
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

void OptionsMenu::tick_update(ServiceProvider& svc) {
	constrain_selection();
	for (auto& option : options) { option.update(svc, current_selection); }
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection).left_offset);
	right_dot.set_target_position(options.at(current_selection).right_offset);

	svc.soundboard.play_sounds(svc);
	svc.controller_map.reset_triggers();
}

void OptionsMenu::frame_update(ServiceProvider& svc) {}

void OptionsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {

	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

// helper
void OptionsMenu::constrain_selection() {
	if (current_selection >= options.size()) { current_selection = 0; }
	if (current_selection < 0) { current_selection = options.size() - 1; }
}

} // namespace automa
