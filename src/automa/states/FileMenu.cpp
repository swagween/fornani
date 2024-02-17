
#include "FileMenu.hpp"

namespace automa {

FileMenu::FileMenu() {
	file_selection = 0;
	state = STATE::STATE_FILE;
	svc::dataLocator.get().load_blank_save();
	hud.set_corner_pad(true); // display hud preview for each file in the center of the screen
	constrain_selection();
	svc::dataLocator.get().load_progress(file_selection);
	svc::cameraLocator.get().set_position({1, 1});
	svc::playerLocator.get().set_position({(float)(cam::screen_dimensions.x / 2) + 80, 360});

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

	for (int i = 0; i < num_files; ++i) {
		file_rects.at(i) =
			sf::IntRect({text_left, top_buffer + (text_dim.y * (i)) + (selection_buffer * (i % num_files))}, text_dim);
	}

	left_dot =
		vfx::Attractor({file_rects.at(0).getPosition().x - dot_pad.x, file_rects.at(0).getPosition().y + dot_pad.y},
					   flcolor::bright_orange, 0.008f);
	right_dot = vfx::Attractor({file_rects.at(0).getPosition().x + file_rects.at(0).width + dot_pad.x,
								file_rects.at(0).getPosition().y + dot_pad.y},
							   flcolor::bright_orange, 0.008f);

	left_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.83f, 0.83f}, 1.0f);
	left_dot.collider.physics.maximum_velocity = sf::Vector2<float>(4.5f, 4.5f);
	right_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.83f, 0.83f}, 1.0f);
	right_dot.collider.physics.maximum_velocity = sf::Vector2<float>(4.5f, 4.5f);

	left_dot.collider.bounding_box.set_position(static_cast<sf::Vector2<float>>(file_rects.at(0).getPosition()));
	right_dot.collider.bounding_box.set_position(
		static_cast<sf::Vector2<float>>(file_rects.at(0).getPosition() + file_rects.at(0).getSize()));
	left_dot.collider.physics.position = (static_cast<sf::Vector2<float>>(file_rects.at(0).getPosition()));
	right_dot.collider.physics.position =
		(static_cast<sf::Vector2<float>>(file_rects.at(0).getPosition() + file_rects.at(0).getSize()));

	logic();

	for (auto i = 0; i < num_files * 2; ++i) {

		file_text.at(i) = sf::Sprite{svc::assetLocator.get().t_file_text, sf::IntRect({0, i * text_dim.y}, text_dim)};
		file_text.at(i).setPosition(text_left,
									top_buffer + (text_dim.y * (i % num_files)) + (selection_buffer * (i % num_files)));
	}
}

FileMenu::~FileMenu() { svc::playerLocator.get().total_reset(); }

void FileMenu::init(std::string const& load_path) {}

void FileMenu::setTilesetTexture(sf::Texture& t) {}

void FileMenu::handle_events(sf::Event& event) {

	if (event.type == sf::Event::EventType::KeyPressed) {
		if (event.key.code == sf::Keyboard::Down) {
			++file_selection;
			constrain_selection();
			svc::dataLocator.get().load_blank_save();
			svc::dataLocator.get().load_progress(file_selection);
			svc::assetLocator.get().menu_shift.play();
		}
		if (event.key.code == sf::Keyboard::Up) {
			--file_selection;
			constrain_selection();
			svc::dataLocator.get().load_blank_save();
			svc::dataLocator.get().load_progress(file_selection);
			svc::assetLocator.get().menu_shift.play();
		}
		if (event.key.code == sf::Keyboard::Left) {
			svc::stateControllerLocator.get().exit_submenu = true;
			svc::assetLocator.get().menu_back.play();
		}
		if (event.key.code == sf::Keyboard::Z || event.key.code == sf::Keyboard::Enter) {
			constrain_selection();
			svc::dataLocator.get().load_progress(file_selection, true);
			svc::stateControllerLocator.get().save_loaded = true;
			svc::assetLocator.get().click.play();
		}
	}
}

void FileMenu::logic() {
	constrain_selection();

	left_dot.update();
	right_dot.update();
	left_dot.set_target_position({text_left - dot_pad.x, file_rects.at(file_selection).getPosition().y + dot_pad.y});
	right_dot.set_target_position({text_right + dot_pad.x, file_rects.at(file_selection).getPosition().y + dot_pad.y});

	hud.update();
	for (auto& a : svc::playerLocator.get().antennae) { a.collider.reset(); }

	svc::playerLocator.get().collider.physics.acceleration = {0.f, 0.f};
	svc::playerLocator.get().collider.physics.velocity = {0.f, 0.f};
	svc::playerLocator.get().collider.physics.zero();
	svc::playerLocator.get().flags.state.set(player::State::alive);
	svc::playerLocator.get().collider.reset();
	svc::playerLocator.get().flags.movement.set(player::Movement::move_left);
	svc::playerLocator.get().collider.flags.set(shape::State::grounded);

	svc::playerLocator.get().behavior.facing_lr = behavior::DIR_LR::LEFT;
	svc::playerLocator.get().update_weapon();
	svc::playerLocator.get().update_animation();
	svc::playerLocator.get().update_sprite();
	svc::playerLocator.get().update_direction();
	svc::playerLocator.get().apparent_position.x = svc::playerLocator.get().collider.physics.position.x + player::PLAYER_WIDTH / 2;
	svc::playerLocator.get().apparent_position.y = svc::playerLocator.get().collider.physics.position.y;
	svc::playerLocator.get().update_behavior();
	svc::playerLocator.get().update_antennae();
}

void FileMenu::render(sf::RenderWindow& win) {
	win.draw(title);
	svc::counterLocator.get().at(svc::draw_calls)++;

	int selection_adjustment{};
	for (auto i = 0; i < num_files; ++i) {
		if (i == file_selection) {
			selection_adjustment = 3;
		} else {
			selection_adjustment = 0;
		}
		win.draw(file_text.at(i + selection_adjustment));
		svc::counterLocator.get().at(svc::draw_calls)++;
	}

	left_dot.render(win, {0, 0});
	right_dot.render(win, {0, 0});

	svc::playerLocator.get().render(win, svc::cameraLocator.get().physics.position);

	hud.render(win);
}

// helper
void FileMenu::constrain_selection() {
	if (file_selection >= num_files) { file_selection = 0; }
	if (file_selection < 0) { file_selection = num_files - 1; }
}

}
