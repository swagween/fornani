
#include "Spring.hpp"
#include "../service/ServiceProvider.hpp"

namespace vfx {

Spring::Spring(Parameters params) : params(params) {
	variables.physics.set_constant_friction({params.dampen_factor, params.dampen_factor});
	variables.physics.maximum_velocity = {6.f, 6.f};
}

void Spring::calculate() { calculate_force(); }

void Spring::update(automa::ServiceProvider& svc) {
	variables.physics.gravity = 1.5f;
	calculate();
	variables.physics.update(svc);
	bob = variables.physics.position;
}

void Spring::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	bob_shape.setRadius(8.f);
	anchor_shape.setRadius(6.f);
	bob_shape.setOrigin({bob_shape.getRadius(), bob_shape.getRadius()});
	anchor_shape.setOrigin({anchor_shape.getRadius(), anchor_shape.getRadius()});
	bob_shape.setFillColor(sf::Color::Transparent);
	anchor_shape.setFillColor(sf::Color::Transparent);
	bob_shape.setPosition(bob - cam);
	anchor_shape.setPosition(anchor - cam);
	bob_shape.setOutlineThickness(-2);
	anchor_shape.setOutlineThickness(-2);
	bob_shape.setOutlineColor(sf::Color::Green);
	anchor_shape.setOutlineColor(sf::Color::Yellow);
	win.draw(bob_shape);
	win.draw(anchor_shape);
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

sf::Vector2<float> Spring::get_rope(int index) {
	auto ret = sf::Vector2<float>{};
	ret = (bob - anchor) / (float)num_links;
	ret = bob - ret * (float)index;
	return ret;
}

} // namespace vfx
