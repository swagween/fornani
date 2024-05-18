#pragma once

#include "CircleSensor.hpp"
#include <algorithm>

namespace components {
CircleSensor::CircleSensor() {
	bounds.setRadius(32);
	bounds.setOutlineColor(sf::Color::White);
	bounds.setOutlineThickness(-1);
	bounds.setFillColor(sf::Color::Transparent);
}
CircleSensor::CircleSensor(float radius) {
	bounds.setRadius(radius);
	bounds.setOutlineColor(sf::Color::White);
	bounds.setOutlineThickness(-1);
	bounds.setFillColor(sf::Color::Transparent);
}
bool CircleSensor::within_bounds(shape::Shape& rect) const {
	auto x = std::clamp(bounds.getPosition().x, rect.position.x, rect.position.x + rect.dimensions.x);
	auto y = std::clamp(bounds.getPosition().y, rect.position.y, rect.position.y + rect.dimensions.y);
	sf::Vector2<float> closest = {x, y};
	return rect.getLength(closest - bounds.getPosition()) < bounds.getRadius();
}
} // namespace components
