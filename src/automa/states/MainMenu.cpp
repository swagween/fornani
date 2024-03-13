
#include "MainMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

MainMenu::MainMenu(ServiceProvider& svc, int id) {
	state = STATE::STATE_MENU;
	svc::cameraLocator.get().set_position({1, 1});

	selection_width = 92;
	selection_buffer = 14;
	title_buffer = 70;
	top_buffer = 296;
	middle = (int)cam::screen_dimensions.x / 2;
	int selection_point = middle - selection_width / 2;

	new_rect = {{middle - 20, top_buffer}, {40, 10}};
	load_rect = {{middle - 24, top_buffer + selection_buffer + new_rect.getSize().y}, {50, 16}};
	options_rect = {{middle - 46, top_buffer + (selection_buffer * 2) + new_rect.getSize().y + load_rect.getSize().y},
					{92, 22}};

	left_dot = vfx::Gravitator({new_rect.getPosition().x - dot_pad.x, new_rect.getPosition().y + dot_pad.y},
							  flcolor::bright_orange, dot_force);
	right_dot = vfx::Gravitator({new_rect.getPosition().x + new_rect.width + dot_pad.x, new_rect.getPosition().y + dot_pad.y},
					   flcolor::bright_orange, dot_force);

	left_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{dot_fric, dot_fric}, 1.0f);
	left_dot.collider.physics.maximum_velocity = sf::Vector2<float>(dot_speed, dot_speed);
	right_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{dot_fric, dot_fric}, 1.0f);
	right_dot.collider.physics.maximum_velocity = sf::Vector2<float>(dot_speed, dot_speed);

	left_dot.collider.bounding_box.set_position(static_cast<sf::Vector2<float>>(new_rect.getPosition()));
	right_dot.collider.bounding_box.set_position(
		static_cast<sf::Vector2<float>>(new_rect.getPosition() + new_rect.getSize()));
	left_dot.collider.physics.position = (static_cast<sf::Vector2<float>>(new_rect.getPosition()));
	right_dot.collider.physics.position =
		(static_cast<sf::Vector2<float>>(new_rect.getPosition() + new_rect.getSize()));

	int y_height_counter{0};
	for (auto i = 0; i < 6; ++i) {

		// the menu options have different sprite heights
		int height{};
		switch (i % 3) {
		case 0: height = 10; break;
		case 1: height = 16; break;
		case 2: height = 22; break;
		}

		title_assets.push_back(sf::Sprite{svc::assetLocator.get().t_title_assets,
										  sf::IntRect({0, y_height_counter}, {selection_width, height})});

		switch (i % 3) {
		case 0: title_assets.at(i).setPosition(selection_point, new_rect.getPosition().y); break;
		case 1: title_assets.at(i).setPosition(selection_point, load_rect.getPosition().y); break;
		case 2: title_assets.at(i).setPosition(selection_point, options_rect.getPosition().y); break;
		}

		y_height_counter += height;
	}

	title = sf::Sprite{svc::assetLocator.get().t_title,
					   sf::IntRect({0, 0}, {(int)cam::screen_dimensions.x, (int)cam::screen_dimensions.y})};
};

void MainMenu::init(ServiceProvider& svc, std::string const& load_path) {
	svc::musicPlayerLocator.get().load("clay");
	svc::musicPlayerLocator.get().play_looped();
}

void MainMenu::setTilesetTexture(ServiceProvider& svc, sf::Texture& t) {}

void MainMenu::handle_events(ServiceProvider& svc, sf::Event& event) {

	if (event.type == sf::Event::EventType::KeyPressed) {
		if (event.key.code == sf::Keyboard::Down) {
			selection = (menu_selection_id.at(selection) % 3 == 2)
							? MenuSelection::new_game
							: (MenuSelection)(menu_selection_id.at(selection) + 1);
			svc::soundboardLocator.get().flags.menu.set(audio::Menu::shift);
		}
		if (event.key.code == sf::Keyboard::Up) {
			selection = (menu_selection_id.at(selection) % 3 == 0)
							? MenuSelection::options
							: (MenuSelection)(menu_selection_id.at(selection) - 1);

			svc::soundboardLocator.get().flags.menu.set(audio::Menu::shift);
		}
		if (event.key.code == sf::Keyboard::Z || event.key.code == sf::Keyboard::Enter) {
			if (selection == MenuSelection::new_game) {

				svc.state_controller.next_state = svc::dataLocator.get().load_blank_save(true);
				svc.state_controller.actions.set(Actions::trigger);
				svc.state_controller.actions.set(Actions::save_loaded);
				svc::soundboardLocator.get().flags.menu.set(audio::Menu::select);
			}
			if (selection == MenuSelection::load_game) {

				svc.state_controller.submenu = menu_type::file_select;
				svc.state_controller.actions.set(Actions::trigger_submenu);
				svc::soundboardLocator.get().flags.menu.set(audio::Menu::select);
			}
			if (selection == MenuSelection::options) {

				// todo: make options menu
				svc::soundboardLocator.get().flags.menu.set(audio::Menu::select);
			}
		}
		if (event.key.code == sf::Keyboard::Right && selection == MenuSelection::load_game) {
			svc.state_controller.submenu = menu_type::file_select;
			svc.state_controller.actions.set(Actions::trigger_submenu);
			svc::soundboardLocator.get().flags.menu.set(audio::Menu::forward_switch);
		}
	}
}

void MainMenu::tick_update(ServiceProvider& svc) {
	svc::musicPlayerLocator.get().update();
	left_dot.update();
	right_dot.update();
	switch (selection) {
	case MenuSelection::new_game:
		dot_pad.y = 5.f;
		left_dot.set_target_position({new_rect.getPosition().x - dot_pad.x, new_rect.getPosition().y + dot_pad.y});
		right_dot.set_target_position(
			{new_rect.getPosition().x + new_rect.width + dot_pad.x, new_rect.getPosition().y + dot_pad.y});
		break;
	case MenuSelection::load_game:
		dot_pad.y = 8.f;
		left_dot.set_target_position({load_rect.getPosition().x - dot_pad.x, load_rect.getPosition().y + dot_pad.y});
		right_dot.set_target_position(
			{load_rect.getPosition().x + load_rect.width + dot_pad.x, load_rect.getPosition().y + dot_pad.y});
		break;
	case MenuSelection::options:
		dot_pad.y = 11.f;
		left_dot.set_target_position(
			{options_rect.getPosition().x - dot_pad.x, options_rect.getPosition().y + dot_pad.y});
		right_dot.set_target_position(
			{options_rect.getPosition().x + options_rect.width + dot_pad.x, options_rect.getPosition().y + dot_pad.y});
		break;
	}
}

void MainMenu::frame_update(ServiceProvider& svc) {}

void MainMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(title);
	

	int selection_adjustment{};
	for (auto i = 0; i < 3; ++i) {
		if (i == menu_selection_id.at(selection)) {
			selection_adjustment = 3;
		} else {
			selection_adjustment = 0;
		}
		if (i + selection_adjustment < 6) {
			win.draw(title_assets.at(i + selection_adjustment));
			
		}
	}

	left_dot.render(win, {0, 0});
	right_dot.render(win, {0, 0});
}

} // namespace automa
