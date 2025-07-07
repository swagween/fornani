#include "fornani/entities/atmosphere/Firefly.hpp"
#include <numbers>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::vfx {

Firefly::Firefly(automa::ServiceProvider& svc, sf::Vector2f start) : sprite(svc.assets.get_texture("firefly"), {9, 9}) {
	physics.set_global_friction(0.99f);
	physics.position = start;
	physics.velocity = util::random::random_vector_float(-1.f, 1.f);
	sprite.push_params("invisible", {10, 1, 32, -1});
	sprite.push_params("glowing", {0, 10, 24, 0});
	sprite.set_params("invisible");
	auto rand_time = util::random::random_range(100, 500);
	light = util::Cooldown(rand_time);
	auto offset = util::random::random_range(0, light.get_native_time());
	light.start(offset);
	sprite.set_origin({4.5f, 4.5f});
	variant = util::random::percent_chance(60) ? 0 : util::random::percent_chance(50) ? 1 : util::random::percent_chance(50) ? 2 : 3;
	if (variant == 0 && util::random::percent_chance(30)) {
		trail = std::make_unique<graphics::SpriteHistory>();
		trail.value()->set_sample_size(12);
	}
}

void Firefly::update(automa::ServiceProvider& svc, world::Map& map) {
	if (!svc.in_window(sprite.get_sprite_position(), sprite.get_dimensions())) { return; }
	light.update();
	steering.smooth_random_walk(physics, 0.003f);
	physics.simple_update();
	map.wrap(physics.position);
	if (light.is_complete()) {
		sprite.set_params("glowing", true);
		light.start();
		glowing = true;
	}
	if (glowing) { light.start(); }
	if (glowing && sprite.complete()) {
		glowing = false;
		sprite.set_params("invisible", true);
	}
	sprite.update(physics.position, variant);
	if (trail && (svc.ticker.every_x_ticks(20) || light.is_almost_complete())) { trail.value()->update(sprite.get_sprite(), physics.position); }
}

void Firefly::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (svc.greyblock_mode()) { return; }
	++svc.out_value;
	if (trail) { trail.value()->drag(win, cam); }
	if (glowing) { sprite.render(svc, win, cam); }
	if (svc.greyblock_mode()) {
		sf::RectangleShape drawbox{};
		drawbox.setSize({2.f, 2.f});
		drawbox.setFillColor(colors::ui_white);
		drawbox.setPosition(physics.position - cam);
		win.draw(drawbox);
		steering.render(svc, win, cam);
	}
}

} // namespace fornani::vfx
