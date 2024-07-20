
#include "Gravitator.hpp"
#include "../service/ServiceProvider.hpp"

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

	// close enough; call them equal and escape
	if (abs(new_position.x - collider.physics.position.x) < 0.1f) { collider.physics.position.x = new_position.x; }
	if (abs(new_position.y - collider.physics.position.y) < 0.1f) { collider.physics.position.y = new_position.y; }
	if (collider.physics.position == new_position) {
		collider.sync_components();
		return;
	}

	float gx = collider.physics.position.x;
	float gy = collider.physics.position.y;
	float mx = new_position.x - collider.bounding_box.dimensions.x / 2;
	float my = new_position.y - collider.bounding_box.dimensions.y / 2;

	float force_x = mx - gx;
	float force_y = my - gy;
	float mag = sqrt((force_x * force_x) + (force_y * force_y));
	mag = std::max(0.0001f, mag);
	float str = attraction_force / mag * mag;
	force_x *= str;
	force_y *= str;

	collider.physics.apply_force({force_x, force_y});
}

void Gravitator::render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos, int history) {

	//just for antennae, can be improved a lot
	auto prev_color = box.getFillColor();
	if(history > 0) {
		box.setFillColor(svc.styles.colors.fucshia);
		win.draw(box);
	}

	box.setFillColor(prev_color);
	box.setPosition(collider.bounding_box.position.x - campos.x,
					collider.bounding_box.position.y - campos.y);

	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		win.draw(box);
	} else {
		win.draw(box);
	}
	
}
} // namespace vfx
