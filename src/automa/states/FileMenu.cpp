
#include "FileMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

FileMenu::FileMenu(ServiceProvider& svc, player::Player& player, int id) : GameState(svc, player, id) {
	file_selection = 0;
	state = STATE::STATE_FILE;
	svc.data.load_blank_save(player);
	hud.set_corner_pad(svc, true); // display hud preview for each file in the center of the screen
	constrain_selection();
	svc.state_controller.next_state = svc.data.load_progress(player, file_selection);
	svc::cameraLocator.get().set_position({1, 1});
	player.set_position({(float)(cam::screen_dimensions.x / 2) + 80, 360});

	title.setPosition(0, 0);
	title.setSize(static_cast<sf::Vector2f>(cam::screen_dimensions));
	title.setFillColor(flcolor::ui_black);

	selection_width = 92;
	selection_buffer = 14;
	top_buffer = 186;
	middle = (int)cam::screen_dimensions.x / 2;
	int selection_point = middle - selection_width / 2;
	text_left = middle - text_dim.x / 2;
	text_right = middle + text_dim.x / 2;

	for (int i = 0; i < num_files; ++i) { file_rects.at(i) = sf::IntRect({text_left, top_buffer + (text_dim.y * (i)) + (selection_buffer * (i % num_files))}, text_dim); }

	left_dot = vfx::Gravitator({file_rects.at(0).getPosition().x - dot_pad.x, file_rects.at(0).getPosition().y + dot_pad.y}, flcolor::bright_orange, dot_force);
	right_dot = vfx::Gravitator({file_rects.at(0).getPosition().x + file_rects.at(0).width + dot_pad.x, file_rects.at(0).getPosition().y + dot_pad.y}, flcolor::bright_orange, dot_force);

	left_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{dot_fric, dot_fric}, 1.0f);
	left_dot.collider.physics.maximum_velocity = sf::Vector2<float>(dot_speed, dot_speed);
	right_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{dot_fric, dot_fric}, 1.0f);
	right_dot.collider.physics.maximum_velocity = sf::Vector2<float>(dot_speed, dot_speed);

	left_dot.collider.bounding_box.set_position(static_cast<sf::Vector2<float>>(file_rects.at(0).getPosition()));
	right_dot.collider.bounding_box.set_position(static_cast<sf::Vector2<float>>(file_rects.at(0).getPosition() + file_rects.at(0).getSize()));
	left_dot.collider.physics.position = (static_cast<sf::Vector2<float>>(file_rects.at(0).getPosition()));
	right_dot.collider.physics.position = (static_cast<sf::Vector2<float>>(file_rects.at(0).getPosition() + file_rects.at(0).getSize()));

	for (auto i = 0; i < num_files * 2; ++i) {

		file_text.at(i) = sf::Sprite{svc.assets.t_file_text, sf::IntRect({0, i * text_dim.y}, text_dim)};
		file_text.at(i).setPosition(text_left, top_buffer + (text_dim.y * (i % num_files)) + (selection_buffer * (i % num_files)));
	}
}

void FileMenu::init(ServiceProvider& svc, std::string_view room) {}

void FileMenu::setTilesetTexture(ServiceProvider& svc, sf::Texture& t) {}

void FileMenu::handle_events(ServiceProvider& svc, sf::Event& event) {

	svc.controller_map.handle_joystick_events(event);
	if (event.type == sf::Event::EventType::KeyPressed) { svc.controller_map.handle_press(event.key.code); }
	if (event.type == sf::Event::EventType::KeyReleased) { svc.controller_map.handle_release(event.key.code); }

		if (svc.controller_map.label_to_control.at("down").triggered()) {
			++file_selection;
			constrain_selection();
			svc.data.load_blank_save(*player);
			svc.data.load_progress(*player, file_selection);
			svc.soundboard.flags.menu.set(audio::Menu::shift);
		}
		if (svc.controller_map.label_to_control.at("up").triggered()) {
			--file_selection;
			constrain_selection();
			svc.data.load_blank_save(*player);
			svc.data.load_progress(*player, file_selection);
			svc.soundboard.flags.menu.set(audio::Menu::shift);
		}
		if (svc.controller_map.label_to_control.at("left").triggered()) {
			svc.state_controller.actions.set(Actions::exit_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		}
		if (svc.controller_map.label_to_control.at("main_action").triggered()) {
			constrain_selection();
			svc.data.load_progress(*player, file_selection, true);
			svc.state_controller.actions.set(Actions::trigger);
			svc.state_controller.actions.set(Actions::save_loaded);
			svc.soundboard.flags.menu.set(audio::Menu::select);
			svc.soundboard.flags.world.set(audio::World::load);
		}
	if (event.type == sf::Event::EventType::JoystickMoved) { svc.controller_map.reset_triggers(); }
}

void FileMenu::tick_update(ServiceProvider& svc) {
	constrain_selection();

	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position({text_left - dot_pad.x, file_rects.at(file_selection).getPosition().y + dot_pad.y});
	right_dot.set_target_position({text_right + dot_pad.x, file_rects.at(file_selection).getPosition().y + dot_pad.y});

	hud.update(*player);
	for (auto& a : player->antennae) { a.collider.reset(); }

	player->collider.physics.acceleration = {};
	player->collider.physics.velocity = {};
	player->collider.physics.zero();
	player->flags.state.set(player::State::alive);
	player->collider.reset();
	player->controller.autonomous_walk();
	player->collider.flags.set(shape::State::grounded);

	player->update_weapon();
	player->update_animation();
	player->update_sprite();
	player->update_direction();
	player->apparent_position.x = player->collider.physics.position.x + player::PLAYER_WIDTH / 2;
	player->apparent_position.y = player->collider.physics.position.y;
	player->update_animation();
	player->update_antennae();

	svc.soundboard.play_sounds(svc);
	svc.controller_map.reset_triggers();
}

void FileMenu::frame_update(ServiceProvider& svc) {}

void FileMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	win.draw(title);
	

	int selection_adjustment{};
	for (auto i = 0; i < num_files; ++i) {
		if (i == file_selection) {
			selection_adjustment = 3;
		} else {
			selection_adjustment = 0;
		}
		win.draw(file_text.at(i + selection_adjustment));
		
	}

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});

	player->render(svc, win, svc::cameraLocator.get().physics.position);

	hud.render(*player, win);
}

// helper
void FileMenu::constrain_selection() {
	if (file_selection >= num_files) { file_selection = 0; }
	if (file_selection < 0) { file_selection = num_files - 1; }
}

} // namespace automa
