
#include <fornani/particle/Gravitator.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::vfx {

Gravitator::Gravitator(sf::Vector2f pos, sf::Color col, float agf, sf::Vector2f size) : dimensions(size), color(col), attraction_force(agf) {
	collider.physics.position = pos * constants::f_cell_size;
	collider.set_position(pos);
	box.setSize(dimensions);
	box.setPosition(collider.get_global_center());
	box.setFillColor(color);
	box.setOrigin(box.getLocalBounds().size * 0.5f);
}

void Gravitator::update(automa::ServiceProvider& svc) { collider.physics.update_dampen(svc); }

void Gravitator::add_force(sf::Vector2f force) { collider.physics.apply_force(force); }

void Gravitator::set_position(sf::Vector2f new_position) { collider.set_position(new_position); }

void Gravitator::set_target_position(sf::Vector2f new_position) { steering.target(collider.physics, new_position, attraction_force); }

void Gravitator::demagnetize(automa::ServiceProvider& svc) {
	collider.physics.set_global_friction(0.99f);
	collider.physics.gravity = 6.f;
	collider.physics.elasticity = 1.f;
}

void Gravitator::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos, int history) {
	// just for antennae, can be improved a lot
	auto prev_color = box.getFillColor();
	if (history > 0) {
		box.setFillColor(colors::fucshia);
		win.draw(box);
	}

	box.setFillColor(prev_color);
	box.setPosition(collider.get_global_center() - campos);

	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		win.draw(box);
	} else {
		win.draw(box);
	}
}

} // namespace fornani::vfx
