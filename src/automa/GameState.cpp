
#include "GameState.hpp"
#include "../service/ServiceProvider.hpp"

namespace automa {

Option::Option(ServiceProvider& svc, std::string_view lbl) {
	label.setString(lbl.data());
	label.setCharacterSize(16);
	label.setLetterSpacing(title_letter_spacing);
	label.setFillColor(svc.styles.colors.ui_white);
	label.setOrigin(label.getLocalBounds().width * 0.5f, label.getLocalBounds().height * 0.5f);
}

void Option::update(ServiceProvider& svc, int& selection) {
	label.setPosition(position);
	left_offset = position - sf::Vector2<float>{label.getLocalBounds().width * 0.5f + dot_offset.x - 2, -dot_offset.y};
	right_offset = position + sf::Vector2<float>{label.getLocalBounds().width * 0.5f + dot_offset.x, dot_offset.y};
	selection == index ? label.setFillColor(svc.styles.colors.ui_white) : label.setFillColor(svc.styles.colors.dark_grey);
	if (flagged) { label.setFillColor(svc.styles.colors.red); }
	label.setOrigin(label.getLocalBounds().width * 0.5f, label.getLocalBounds().height * 0.5f);
}
GameState::GameState(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : player(&player), hud(svc, player, {20, 20}), inventory_window(svc), scene(scene) {
	font.loadFromFile(svc.text.title_font);
	font.setSmooth(false);
	subtitle_font.loadFromFile(svc.text.text_font);
	subtitle_font.setSmooth(false);
	auto const& in_data = svc.data.menu["options"];
	for (auto& entry : in_data[scene].array_view()) {
		options.push_back(Option(svc, entry.as_string()));
	}
	top_buffer = svc.data.menu["config"][scene]["top_buffer"].as<float>();
	int ctr{};
	for (auto& option : options) {
		option.position = {svc.constants.screen_dimensions.x * 0.5f - option.label.getLocalBounds().width, top_buffer + ctr * (option.label.getLocalBounds().height + spacing)};
		option.label.setFont(font);
		option.index = ctr;
		option.update(svc, current_selection);
		++ctr;
	}
	if (options.empty()) { return; }
	left_dot = vfx::Gravitator({options.at(0).position.x - dot_pad.x, options.at(0).position.y + dot_pad.y}, flcolor::bright_orange, dot_force);
	right_dot = vfx::Gravitator({options.at(0).position.x + options.at(0).label.getLocalBounds().width + dot_pad.x, options.at(0).position.y + dot_pad.y}, flcolor::bright_orange, dot_force);

	left_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{dot_fric, dot_fric}, 1.0f);
	left_dot.collider.physics.maximum_velocity = sf::Vector2<float>(dot_speed, dot_speed);
	right_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{dot_fric, dot_fric}, 1.0f);
	right_dot.collider.physics.maximum_velocity = sf::Vector2<float>(dot_speed, dot_speed);
}
// helper
void GameState::constrain_selection() {
	if (current_selection >= (int)options.size()) { current_selection = 0; }
	if (current_selection < 0) { current_selection = options.size() - 1; }
}
} // namespace automa