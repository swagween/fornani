#include "fornani/graphics/Droplet.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"

#include <numbers>

#include "fornani/utils/Random.hpp"

namespace fornani::vfx {

Droplet::Droplet(sf::Vector2<float> start, DropParams params, float tweak) : collider(shape::CircleCollider(3.f)), params(params) {
	collider.physics.position = start;
	collider.physics.velocity.x = params.slant + tweak;
	collider.physics.gravity = 8.f;
	collider.physics.elasticity = 0.1f;
	collider.physics.set_constant_friction({0.8f, params.fall_speed});
}

void Droplet::update(automa::ServiceProvider& svc, world::Map& map, bool collision) {
	if (collision) {
		post_collision.update();
		counter.update();
		collider.update(svc);
		collider.handle_map_collision(map);
		collider.physics.acceleration = {};
	}
	auto const slope = position() - collider.physics.previous_position;
	auto const mag = util::magnitude(slope);
	auto const adjacent = collider.physics.previous_position.x - position().x;
	angle = static_cast<float>(tan(adjacent / mag) * 180.0 / std::numbers::pi);
	if (collider.collided() && collision) {
		decay();
		collider.physics.velocity.x *= 0.9f;
	} else {
		auto const offset = util::Random::random_range_float(0.f, static_cast<float>(std::numbers::pi));
		collider.physics.position.x += sin(counter.get_count() * 0.01f + offset) * params.sway;
	}
}

void Droplet::decay() {
	if (!post_collision.running()) { post_collision.start(); }
}

} // namespace fornani::vfx
