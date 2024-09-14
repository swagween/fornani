#include "FloatingPart.hpp"
#include "../../service/ServiceProvider.hpp"
#include <algorithm>
#include <numbers>

namespace entity {

FloatingPart::FloatingPart(sf::Texture& tex, float force, float friction, sf::Vector2<float> offset) {
	sprite.setTexture(tex);
	sprite.setOrigin(sprite.getLocalBounds().getSize() * 0.5f);
	gravitator = std::make_unique<vfx::Gravitator>(sf::Vector2<float>{}, sf::Color::Transparent, force);
	gravitator->collider.physics = components::PhysicsComponent(sf::Vector2<float>{friction, friction}, 1.0f);
	gravitator->collider.physics.maximum_velocity = sf::Vector2<float>(20.f, 20.f);
	left = offset;
	right = offset;
	right.x *= -1.f;
	init = true;
}

void FloatingPart::update(automa::ServiceProvider& svc, dir::Direction direction, sf::Vector2<float> scale, sf::Vector2<float> position) {
	if (init) {
		gravitator->set_position(position + actual);
		movement.time = svc.random.random_range_float(0.f, 2.f * std::numbers::pi);
		init = false;
	}
	actual = direction.lr == dir::LR::left ? position + left : position + right;
	movement.time += movement.rate;
	auto tweak = movement.magnitude * std::sin(movement.time);
	if (tweak == 0.f) { movement.time = 0.f; }
	actual.y += tweak;
	gravitator->set_target_position(actual);
	gravitator->update(svc);
	sprite.setScale(scale);
}

void FloatingPart::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	sprite.setPosition(gravitator->position() - cam);
	win.draw(sprite);
}

} // namespace entity