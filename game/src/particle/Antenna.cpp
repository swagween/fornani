
#include <fornani/particle/Antenna.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::vfx {

Antenna::Antenna(world::Map& map, sf::Vector2f pos, sf::Color col, float agf, sf::Vector2f size) : m_collider{map, dimensions.x * 0.5f}, dimensions(size), color(col), attraction_force(agf) {
	get_collider().physics.position = pos * constants::f_cell_size;
	get_collider().set_position(pos);
	box.setSize(dimensions);
	box.setPosition(get_collider().get_global_center());
	box.setFillColor(color);
	box.setOrigin(box.getLocalBounds().size * 0.5f);
	get_collider().set_trait(shape::ColliderTrait::circle);
	get_collider().set_trait(shape::ColliderTrait::particle);
	get_collider().set_exclusion_target(shape::ColliderTrait::player);
	get_collider().set_exclusion_target(shape::ColliderTrait::enemy);
	get_collider().set_exclusion_target(shape::ColliderTrait::block);
	get_collider().set_attribute(shape::ColliderAttributes::no_collision);
	get_collider().set_attribute(shape::ColliderAttributes::custom_resolution);
}

void Antenna::update(automa::ServiceProvider& svc) { get_collider().physics.update_dampen(svc); }

void Antenna::add_force(sf::Vector2f force) { get_collider().physics.apply_force(force); }

void Antenna::set_position(sf::Vector2f new_position) { get_collider().set_position(new_position); }

void Antenna::set_target_position(sf::Vector2f new_position) { steering.target(get_collider().physics, new_position, attraction_force); }

void Antenna::demagnetize(automa::ServiceProvider& svc) {
	get_collider().physics.set_friction_componentwise({0.995f, 0.995f});
	get_collider().physics.gravity = 2.f;
	get_collider().physics.elasticity = 0.95f;
	get_collider().set_attribute(shape::ColliderAttributes::no_collision, false);
	get_collider().set_attribute(shape::ColliderAttributes::custom_resolution, false);
}

void Antenna::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos, int history) {
	auto prev_color = box.getFillColor();
	if (history > 0) {
		box.setFillColor(colors::fucshia);
		win.draw(box);
	}

	box.setFillColor(prev_color);
	box.setPosition(get_collider().get_global_center() - campos);

	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		win.draw(box);
	} else {
		win.draw(box);
	}
}

} // namespace fornani::vfx
