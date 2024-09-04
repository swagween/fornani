
#include "GameState.hpp"
#include "../service/ServiceProvider.hpp"

namespace automa {

GameState::GameState(ServiceProvider& svc, player::Player& player, std::string_view scene, int id)
	: player(&player), hud(svc, player, {20, 20}), inventory_window(svc), scene(scene), pause_window(svc) {
	font.loadFromFile(svc.text.title_font);
	font.setSmooth(false);
	subtitle_font.loadFromFile(svc.text.text_font);
	subtitle_font.setSmooth(false);
	auto const& in_data = svc.data.menu["options"];
	for (auto& entry : in_data[scene].array_view()) { options.push_back(Option(svc, entry.as_string(), font)); }
	if (!options.empty()) { current_selection = util::Circuit(static_cast<int>(options.size())); }

	top_buffer = svc.data.menu["config"][scene]["top_buffer"].as<float>();
	int ctr{};
	for (auto& option : options) {
		option.position = {svc.constants.screen_dimensions.x * 0.5f, top_buffer + ctr * spacing};
		option.index = ctr;
		option.update(svc, current_selection.get());
		++ctr;
	}
	if (options.empty()) { return; }
	left_dot = vfx::Gravitator({options.at(0).position.x - dot_pad.x, options.at(0).position.y + dot_pad.y}, svc.styles.colors.bright_orange, dot_force);
	right_dot = vfx::Gravitator({options.at(0).position.x + options.at(0).label.getLocalBounds().width + dot_pad.x, options.at(0).position.y + dot_pad.y}, svc.styles.colors.bright_orange, dot_force);

	left_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{dot_fric, dot_fric}, 1.0f);
	left_dot.collider.physics.maximum_velocity = sf::Vector2<float>(dot_speed, dot_speed);
	right_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{dot_fric, dot_fric}, 1.0f);
	right_dot.collider.physics.maximum_velocity = sf::Vector2<float>(dot_speed, dot_speed);
}

} // namespace automa