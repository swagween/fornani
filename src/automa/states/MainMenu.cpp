
#include "MainMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

MainMenu::MainMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {
	state = STATE::STATE_MENU;
	svc::cameraLocator.get().set_position({1, 1});

	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);

	title = sf::Sprite{svc.assets.t_title, sf::IntRect({0, 0}, {(int)cam::screen_dimensions.x, (int)cam::screen_dimensions.y})};
};

void MainMenu::init(ServiceProvider& svc, std::string_view room) {
	svc::musicPlayerLocator.get().load("clay");
	svc::musicPlayerLocator.get().play_looped();
}

void MainMenu::setTilesetTexture(ServiceProvider& svc, sf::Texture& t) {}

void MainMenu::handle_events(ServiceProvider& svc, sf::Event& event) {
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
	if (svc.controller_map.label_to_control.at("main_action").triggered()) {
		if (current_selection == menu_selection_id.at(MenuSelection::play)) {
			svc.state_controller.submenu = menu_type::file_select;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection == menu_selection_id.at(MenuSelection::options)) {
			svc.state_controller.submenu = menu_type::options;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		}
		if (current_selection == menu_selection_id.at(MenuSelection::quit)) {
			svc.state_controller.actions.set(Actions::shutdown);
		}
	}
	if (!svc.controller_map.is_gamepad() && svc.controller_map.label_to_control.at("right").triggered()) {
		if (current_selection == menu_selection_id.at(MenuSelection::play)) { svc.state_controller.submenu = menu_type::file_select; }
		if (current_selection == menu_selection_id.at(MenuSelection::options)) { svc.state_controller.submenu = menu_type::options; }
		svc.state_controller.actions.set(Actions::trigger_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
	}
	svc.controller_map.reset_triggers();
}

void MainMenu::tick_update(ServiceProvider& svc) {
	for (auto& option : options) { option.update(svc, current_selection); }
	svc::musicPlayerLocator.get().update(svc);
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection).left_offset);
	right_dot.set_target_position(options.at(current_selection).right_offset);
	svc.soundboard.play_sounds(svc);
	svc.controller_map.reset_triggers();
}

void MainMenu::frame_update(ServiceProvider& svc) {}

void MainMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(title);

	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
