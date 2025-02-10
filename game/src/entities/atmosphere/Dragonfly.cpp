#include "fornani/entities/atmosphere/Dragonfly.hpp"
#include <numbers>
#include "fornani/entities/player/Player.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"

#include "fornani/utils/Random.hpp"

namespace fornani::vfx {

Dragonfly::Dragonfly(automa::ServiceProvider& svc, sf::Vector2<float> start) : sprite(svc.assets.t_dragonfly, {16, 16}) {
	physics.set_global_friction(0.97f);
	physics.position = start;
	physics.velocity = util::Random::random_vector_float(-1.f, 1.f);
	sprite.push_params("neutral", {0, 3, 16, -1});
	sprite.set_params("neutral");
	sprite.random_start();
	sprite.set_origin({12.f, 12.f});
	variant = util::Random::percent_chance(60) ? 0 : util::Random::percent_chance(50) ? 1 : util::Random::percent_chance(50) ? 2 : 3;
	forces.seek = 0.000052f;
	forces.walk = 0.0113f;
	forces.evade = 0.873f;
}

void Dragonfly::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (!svc.in_window(sprite.get_sprite_position(), sprite.get_dimensions())) { return; }
	steering.evade(physics, player.collider.get_center(), forces.evade, 0.05f);
	steering.seek(physics, map.get_nearest_target_point(physics.position), forces.seek);
	steering.smooth_random_walk(physics, forces.walk);
	physics.simple_update();
	sprite.update(physics.position, variant);
}

void Dragonfly::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) { return; }
	sprite.render(svc, win, cam);
	if (svc.greyblock_mode()) {
		sf::RectangleShape drawbox{};
		drawbox.setSize({2.f, 2.f});
		drawbox.setFillColor(svc.styles.colors.ui_white);
		drawbox.setPosition(physics.position - cam);
		win.draw(drawbox);
		steering.render(svc, win, cam);
	}
}

void Dragonfly::set_forces(float seek, float walk, float evade) { forces = {seek, walk, evade}; }

void Dragonfly::debug() {}

} // namespace fornani::vfx