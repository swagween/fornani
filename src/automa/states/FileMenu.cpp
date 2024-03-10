
#include "FileMenu.hpp"

namespace automa {

FileMenu::FileMenu(services::ServiceLocator& svc) {
	file_selection = 0;
	state = STATE::STATE_FILE;
	svc.dataLocator.get().load_blank_save();
	hud.set_corner_pad(true); // display hud preview for each file in the center of the screen
	constrain_selection();
	svc.dataLocator.get().load_progress(file_selection);
	svc.cameraLocator.get().set_position({1, 1});
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

	tick_update();

	for (auto i = 0; i < num_files * 2; ++i) {

		file_text.at(i) = sf::Sprite{svc.assetLocator.get().t_file_text, sf::IntRect({0, i * text_dim.y}, text_dim)};
		file_text.at(i).setPosition(text_left, top_buffer + (text_dim.y * (i % num_files)) + (selection_buffer * (i % num_files)));
	}
}

FileMenu::~FileMenu() {}

void FileMenu::init(std::string const& load_path, services::ServiceLocator& svc) {}

void FileMenu::setTilesetTexture(sf::Texture& t) {}

void FileMenu::handle_events(sf::Event& event, services::ServiceLocator& svc) {

	if (event.type == sf::Event::EventType::KeyPressed) {
		if (event.key.code == sf::Keyboard::Down) {
			++file_selection;
			constrain_selection();
			svc.dataLocator.get().load_blank_save();
			svc.dataLocator.get().load_progress(file_selection);
			svc.soundboardLocator.get().menu.set(audio::Menu::shift);
		}
		if (event.key.code == sf::Keyboard::Up) {
			--file_selection;
			constrain_selection();
			svc.dataLocator.get().load_blank_save();
			svc.dataLocator.get().load_progress(file_selection);
			svc.soundboardLocator.get().menu.set(audio::Menu::shift);
		}
		if (event.key.code == sf::Keyboard::Left) {
			svc.stateControllerLocator.get().exit_submenu = true;
			svc.soundboardLocator.get().menu.set(audio::Menu::backward_switch);
		}
		if (event.key.code == sf::Keyboard::Z || event.key.code == sf::Keyboard::Enter) {
			constrain_selection();
			svc.dataLocator.get().load_progress(file_selection, true);
			svc.stateControllerLocator.get().save_loaded = true;
			svc.soundboardLocator.get().menu.set(audio::Menu::select);
			svc.soundboardLocator.get().world.set(audio::World::load);
		}
	}
}

void FileMenu::tick_update(services::ServiceLocator& svc) {
	constrain_selection();

	left_dot.update();
	right_dot.update();
	left_dot.set_target_position({text_left - dot_pad.x, file_rects.at(file_selection).getPosition().y + dot_pad.y});
	right_dot.set_target_position({text_right + dot_pad.x, file_rects.at(file_selection).getPosition().y + dot_pad.y});

	hud.update();
	for (auto& a : player.antennae) { a.collider.reset(); }

	player.collider.physics.acceleration = {};
	player.collider.physics.velocity = {};
	player.collider.physics.zero();
	player.flags.state.set(player::State::alive);
	player.collider.reset();
	player.controller.autonomous_walk();
	player.collider.flags.set(shape::State::grounded);

	player.update_weapon();
	player.update_animation();
	player.update_sprite();
	player.update_direction();
	player.apparent_position.x = player.collider.physics.position.x + player::PLAYER_WIDTH / 2;
	player.apparent_position.y = player.collider.physics.position.y;
	player.update_animation();
	player.update_antennae();
}

void FileMenu::frame_update(services::ServiceLocator& svc) {}

void FileMenu::render(sf::RenderWindow& win, services::ServiceLocator& svc) {
	win.draw(title);
	svc.counterLocator.get().at(services::counters::draw_calls)++;

	int selection_adjustment{};
	for (auto i = 0; i < num_files; ++i) {
		if (i == file_selection) {
			selection_adjustment = 3;
		} else {
			selection_adjustment = 0;
		}
		win.draw(file_text.at(i + selection_adjustment));
		svc.counterLocator.get().at(services::counters::draw_calls)++;
	}

	left_dot.render(win, {0, 0});
	right_dot.render(win, {0, 0});

	player.render(win, svc.cameraLocator.get().physics.position);

	hud.render(win);
}

// helper
void FileMenu::constrain_selection() {
	if (file_selection >= num_files) { file_selection = 0; }
	if (file_selection < 0) { file_selection = num_files - 1; }
}

} // namespace automa
