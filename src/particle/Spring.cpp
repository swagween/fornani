
#include "Spring.hpp"
#include "../setup/ServiceLocator.hpp"

namespace vfx {

Spring::Spring(Parameters params) : params(params) {
	variables.physics.set_constant_friction({0.999f, params.dampen_factor});
	variables.physics.maximum_velocity = {6.f, 6.f};
}

void Spring::update() {
	variables.physics.gravity = 1.5f;
	calculate_force();
	variables.physics.update();
	bob = variables.physics.position;
}

void Spring::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	bob_shape.setRadius(8.f);
	anchor_shape.setRadius(4.f);
	bob_shape.setFillColor(sf::Color::Transparent);
	anchor_shape.setFillColor(sf::Color::Transparent);
	bob_shape.setPosition(bob - cam);
	anchor_shape.setPosition(anchor - cam);
	bob_shape.setOutlineThickness(-1);
	anchor_shape.setOutlineThickness(-1);
	bob_shape.setOutlineColor(flcolor::green);
	anchor_shape.setOutlineColor(flcolor::goldenrod);
	win.draw(bob_shape);
	win.draw(anchor_shape);

	anchor_shape.setRadius(10.f);
	anchor_shape.setOutlineColor(sf::Color::White);
	// anchor_shape.setPosition(variables.physics.position - cam);
	// win.draw(anchor_shape);
}

void Spring::calculate_force() {
	variables.spring_force = bob - anchor;

	float mag = sqrt(variables.spring_force.x * variables.spring_force.x + variables.spring_force.y * variables.spring_force.y);
	variables.extension = mag - params.rest_length;

	variables.spring_force /= mag;
	variables.spring_force *= -params.spring_constant * variables.extension;
	variables.physics.acceleration = variables.spring_force;
}

void Spring::reverse_anchor_and_bob() {
	auto temp = anchor;
	anchor = bob;
	bob = temp;
	variables.physics.position = bob;
}

void Spring::set_anchor(sf::Vector2<float> point) { anchor = point; }

void Spring::set_bob(sf::Vector2<float> point) {
	bob = point;
	variables.physics.position = point;
}

void Spring::set_rest_length(float point) { params.rest_length = point; }

void Spring::set_force(float force) { params.spring_constant = force; }

sf::Vector2<float>& Spring::get_bob() { return bob; }

sf::Vector2<float>& Spring::get_anchor() { return anchor; }

} // namespace vfx
