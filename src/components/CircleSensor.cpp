#pragma once

#include "CircleSensor.hpp"
#include <algorithm>

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
	drawable.setFillColor(active() ? sf::Color{20, 160, 160, 100} : sf::Color::Transparent);
	drawable.setPosition(bounds.getPosition() - cam);
	win.draw(drawable);
}

void CircleSensor::set_position(sf::Vector2<float> position) { bounds.setPosition(position); }

bool CircleSensor::within_bounds(shape::Shape& rect) const {
	auto x = std::clamp(bounds.getPosition().x, rect.position.x, rect.position.x + rect.dimensions.x);
	auto y = std::clamp(bounds.getPosition().y, rect.position.y, rect.position.y + rect.dimensions.y);
	sf::Vector2<float> closest = {x, y};
	return rect.getLength(closest - bounds.getPosition()) < bounds.getRadius();
}
} // namespace components
