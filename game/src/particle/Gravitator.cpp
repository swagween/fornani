
#include "fornani/particle/Gravitator.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace vfx {

Gravitator::Gravitator(Vec pos, sf::Color col, float agf, Vec size) : scaled_position(pos), dimensions(size), color(col), attraction_force(agf) {
	collider = shape::Collider(sf::Vector2<float>{4.f, 4.f}, sf::Vector2<float>{pos.x, pos.x});
	collider.bounding_box.dimensions = Vec(4, 4);
	collider.physics.position = static_cast<Vec>(pos) * lookup::unit_size_f;
	collider.bounding_box = shape::Shape(collider.bounding_box.dimensions);
	collider.bounding_box.set_position(static_cast<Vec>(pos));
	box.setSize(dimensions);
	box.setPosition(collider.bounding_box.position);
	box.setFillColor(color);
}

void Gravitator::update(automa::ServiceProvider& svc) {
	collider.reset();
	collider.physics.update_dampen(svc);
	collider.sync_components();
}

void Gravitator::add_force(sf::Vector2<float> force) { collider.physics.apply_force(force); }

void Gravitator::set_position(Vec new_position) {
	collider.physics.position = new_position;
	collider.sync_components();
}

void Gravitator::set_target_position(Vec new_position) {
	steering.target(collider.physics, new_position, attraction_force);
	collider.sync_components();
}

void Gravitator::demagnetize(automa::ServiceProvider& svc) {
	collider.physics.set_global_friction(0.99f);
	collider.physics.gravity = 6.f;
	collider.physics.elasticity = 1.f;
}

void Gravitator::render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos, int history) {
	// just for antennae, can be improved a lot
	auto prev_color = box.getFillColor();
	if (history > 0) {
		box.setFillColor(svc.styles.colors.fucshia);
		win.draw(box);
	}

	box.setFillColor(prev_color);
	box.setPosition(collider.bounding_box.position - campos);

	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		win.draw(box);
	} else {
		win.draw(box);
	}
}

} // namespace vfx
