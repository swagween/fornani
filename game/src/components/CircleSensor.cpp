#include "fornani/components/CircleSensor.hpp"
#include "fornani/utils/Math.hpp"

#include <ccmath/ext/clamp.hpp>
#include <fornani/physics/CircleCollider.hpp>

namespace fornani::components {

CircleSensor::CircleSensor() : CircleSensor(32.f) {}

CircleSensor::CircleSensor(float radius) {
	bounds.setRadius(radius);
	bounds.setOutlineColor(sf::Color::White);
	bounds.setOutlineThickness(-2);
	bounds.setFillColor(sf::Color::Transparent);
	bounds.setOrigin({radius, radius});
	drawable = bounds;
}

void CircleSensor::render(sf::RenderWindow& win, sf::Vector2f cam) {
	drawable = bounds;
	drawable.setOrigin({bounds.getRadius(), bounds.getRadius()});
	drawable.setFillColor(active() ? sf::Color{20, 160, 160, 100} : sf::Color::Transparent);
	drawable.setPosition(bounds.getPosition() - cam);
	win.draw(drawable);
}

void CircleSensor::set_position(sf::Vector2f position) { bounds.setPosition(position); }

bool CircleSensor::within_bounds(shape::Shape const& shape) const {
	if (shape.non_square()) { return shape.circle_SAT(bounds); }
	auto const x = ccm::ext::clamp(bounds.getPosition().x, shape.get_position().x, shape.get_position().x + shape.get_dimensions().x);
	auto const y = ccm::ext::clamp(bounds.getPosition().y, shape.get_position().y, shape.get_position().y + shape.get_dimensions().y);
	sf::Vector2 const closest{x, y};
	return util::magnitude(closest - bounds.getPosition()) < bounds.getRadius();
}

bool CircleSensor::within_bounds(shape::CircleCollider const& shape) const { return (bounds.getGlobalBounds().getCenter() - shape.get_global_center()).length() < bounds.getRadius() + shape.get_radius(); }

sf::Vector2f CircleSensor::get_MTV(shape::Shape const& shape) const { return shape.circle_SAT_MTV(bounds); }

} // namespace fornani::components
