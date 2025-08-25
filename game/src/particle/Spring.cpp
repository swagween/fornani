
#include <ccmath/ext/clamp.hpp>
#include <fornani/particle/Spring.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::vfx {

Spring::Spring(SpringParameters params) : params(params) {
	variables.bob_physics.set_constant_friction({params.dampen_factor, params.dampen_factor});
	variables.bob_physics.maximum_velocity = {60.f, 60.f};
	variables.anchor_physics.set_constant_friction({params.dampen_factor, params.dampen_factor});
	variables.anchor_physics.maximum_velocity = {60.f, 60.f};
	sensor.bounds.setOrigin({sensor.bounds.getRadius(), sensor.bounds.getRadius()});
}

Spring::Spring(SpringParameters params, sf::Vector2f anchor, sf::Vector2f bob) : anchor(anchor), bob(bob) {}

void Spring::calculate() { calculate_force(); }

void Spring::update(automa::ServiceProvider& svc, float custom_grav, sf::Vector2f external_force, bool loose, bool sag) {
	variables.bob_physics.gravity = sag ? custom_grav : 0.f;
	variables.anchor_physics.gravity = sag ? custom_grav : 0.f;
	calculate();
	variables.bob_physics.apply_force(external_force);
	variables.bob_physics.update(svc);
	if (loose) {
		variables.anchor_physics.apply_force(external_force);
		variables.anchor_physics.update(svc);
	}
	bob = variables.bob_physics.position;
	if (loose) { anchor = variables.anchor_physics.position; }
	sensor.bounds.setPosition(bob);
}

void Spring::simulate(float custom_grav, bool loose, bool sag) {
	variables.bob_physics.gravity = sag ? custom_grav : 0.f;
	variables.anchor_physics.gravity = sag ? custom_grav : 0.f;
	calculate();
	variables.bob_physics.simple_update();
	if (loose) { variables.anchor_physics.simple_update(); }
	bob = variables.bob_physics.position;
	if (loose) { anchor = variables.anchor_physics.position; }
	sensor.bounds.setPosition(bob);
}

void Spring::render(sf::RenderWindow& win, sf::Vector2f cam) {
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
	sensor.render(win, cam);
}

void Spring::calculate_force() {
	variables.spring_force = bob - anchor;

	float mag = sqrt(variables.spring_force.x * variables.spring_force.x + variables.spring_force.y * variables.spring_force.y);
	variables.extension = mag - params.rest_length;

	if (mag == 0.f) { return; }

	variables.spring_force /= mag;
	variables.spring_force *= -params.spring_constant * variables.extension;
	variables.spring_force.x = ccm::ext::clamp(variables.spring_force.x, -spring_max, spring_max);
	variables.spring_force.y = ccm::ext::clamp(variables.spring_force.y, -spring_max, spring_max);
	variables.bob_physics.acceleration = variables.spring_force;
	variables.anchor_physics.acceleration = -variables.spring_force;
}

void Spring::reverse_anchor_and_bob() {
	auto temp = anchor;
	anchor = bob;
	bob = temp;
	variables.bob_physics.position = bob;
}

void Spring::set_anchor(sf::Vector2f point) {
	anchor = point;
	variables.anchor_physics.position = point;
}

void Spring::set_bob(sf::Vector2f point) {
	bob = point;
	variables.bob_physics.position = point;
}

void Spring::set_rest_length(float point) { params.rest_length = point; }

void Spring::set_force(float force) { params.spring_constant = force; }

sf::Vector2f& Spring::get_bob() { return bob; }

sf::Vector2f& Spring::get_anchor() { return anchor; }

sf::Vector2f Spring::get_rope(int index) {
	auto ret = sf::Vector2f{};
	ret = (bob - anchor) / (float)num_links;
	ret = bob - ret * (float)index;
	return ret;
}

} // namespace fornani::vfx
