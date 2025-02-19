#include "fornani/components/CircleSensor.hpp"
#include "fornani/utils/Math.hpp"

#include <ccmath/ext/clamp.hpp>

namespace components {

CircleSensor::CircleSensor() {
	bounds.setRadius(32);
	bounds.setOutlineColor(sf::Color::White);
	bounds.setOutlineThickness(-1);
	bounds.setFillColor(sf::Color::Transparent);
	drawable = bounds;
}

CircleSensor::CircleSensor(float radius) {
	bounds.setRadius(radius);
	bounds.setOutlineColor(sf::Color::White);
	bounds.setOutlineThickness(-1);
	bounds.setFillColor(sf::Color::Transparent);
	drawable = bounds;
}

void CircleSensor::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	drawable = bounds;
	drawable.setOrigin({bounds.getRadius(), bounds.getRadius()});
	drawable.setFillColor(active() ? sf::Color{20, 160, 160, 100} : sf::Color::Transparent);
	drawable.setPosition(bounds.getPosition() - cam);
	win.draw(drawable);
}

void CircleSensor::set_position(sf::Vector2<float> position) { bounds.setPosition(position); }

bool CircleSensor::within_bounds(shape::Shape& shape) const {
	if (shape.non_square()) { return shape.circle_SAT(bounds); }
	const auto x = ccm::ext::clamp(bounds.getPosition().x, shape.position.x, shape.position.x + shape.dimensions.x);
	const auto y = ccm::ext::clamp(bounds.getPosition().y, shape.position.y, shape.position.y + shape.dimensions.y);
	const sf::Vector2 closest{x, y};
	return util::magnitude(closest - bounds.getPosition()) < bounds.getRadius();
}

sf::Vector2<float> CircleSensor::get_MTV(shape::Shape& shape) { return shape.circle_SAT_MTV(bounds); }

} // namespace components
