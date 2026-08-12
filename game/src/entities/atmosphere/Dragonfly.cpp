
#include "fornani/entities/atmosphere/Dragonfly.hpp"
#include <fornani/graphics/Renderer.hpp>
#include <numbers>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::vfx {

Dragonfly::Dragonfly(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f start) : sprite(svc.assets.get_texture("dragonfly"), {8, 8}) {
	physics.set_global_friction(0.97f);
	physics.position = start;
	physics.velocity = random::random_vector_float(-1.f, 1.f);
	sprite.push_params("neutral", {0, 3, 16, -1});
	sprite.set_params("neutral");
	sprite.random_start();
	sprite.set_origin({4.f, 4.f});
	sprite.set_scale(constants::f_scale_vec);
	variant = random::percent_chance(60) ? 0 : random::percent_chance(50) ? 1 : random::percent_chance(50) ? 2 : 3;
	forces.seek = 0.000052f;
	forces.walk = 0.0113f;
	forces.evade = 0.873f;
}

void Dragonfly::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (!svc.in_window(sprite.get_sprite_position(), sprite.get_dimensions())) { return; }
	steering.evade(physics, player.get_collider().get_center(), forces.evade, 0.05f);
	steering.seek(physics, map.get_nearest_target_point(physics.position), forces.seek);
	steering.smooth_random_walk(physics, forces.walk);
	physics.simple_update();
	sprite.update(physics.position, variant);
}

void Dragonfly::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (debug::is_production()) {
		sprite.render(win, cam);
	} else {
		sf::RectangleShape drawbox{};
		drawbox.setSize({2.f, 2.f});
		drawbox.setFillColor(colors::ui_white);
		drawbox.setPosition(physics.position - cam);
		win.draw(drawbox);
		steering.render(svc, win, cam);
	}
}

void Dragonfly::submit(Renderer& renderer) {
	auto const pos = physics.position;
	auto const& sprite_ref = sprite.get_sprite();
	auto const& frame = sprite_ref.getTextureRect();
	sf::FloatRect dest{pos, sf::Vector2f{static_cast<float>(frame.size.x), static_cast<float>(frame.size.y)}};
	renderer.submit(sprite_ref.getTexture(), dest, frame, RenderLayer::atmosphere);
}

void Dragonfly::set_forces(float seek, float walk, float evade) { forces = {seek, walk, evade}; }

void Dragonfly::debug() {}

} // namespace fornani::vfx
