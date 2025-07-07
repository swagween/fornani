
#include "fornani/automa/GameState.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

GameState::GameState(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number) : player(&player), hud(svc, player), scene(scene) {
	auto const& in_data = svc.data.menu["options"];
	for (auto& entry : in_data[scene].as_array()) { options.push_back(Option(svc, entry.as_string())); }
	if (!options.empty()) { current_selection = util::Circuit(static_cast<int>(options.size())); }

	top_buffer = svc.data.menu["config"][scene]["top_buffer"].as<float>();
	int ctr{};
	for (auto& option : options) {
		option.position = {svc.window->i_screen_dimensions().x * 0.5f, top_buffer + ctr * spacing};
		option.index = ctr;
		option.update(svc, current_selection.get());
		++ctr;
	}
	if (options.empty()) { return; }
	left_dot = vfx::Gravitator({options.at(0).position.x - dot_pad.x, options.at(0).position.y + dot_pad.y}, colors::bright_orange, dot_force);
	right_dot = vfx::Gravitator({options.at(0).position.x + options.at(0).label.getLocalBounds().size.x + dot_pad.x, options.at(0).position.y + dot_pad.y}, colors::bright_orange, dot_force);

	left_dot.collider.physics = components::PhysicsComponent(sf::Vector2f{dot_fric, dot_fric}, 1.0f);
	left_dot.collider.physics.maximum_velocity = sf::Vector2f(dot_speed, dot_speed);
	right_dot.collider.physics = components::PhysicsComponent(sf::Vector2f{dot_fric, dot_fric}, 1.0f);
	right_dot.collider.physics.maximum_velocity = sf::Vector2f(dot_speed, dot_speed);
}

void GameState::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	if (m_console) {
		if (m_console.value()->exit_requested()) { m_console = {}; }
	}
}

} // namespace fornani::automa
