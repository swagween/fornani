
#include <fornani/core/Debug.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/particle/Effect.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Breakable.hpp>
#include <fornani/world/Map.hpp>
#include <algorithm>

namespace fornani::world {

Breakable::Breakable(automa::ServiceProvider& svc, Map& map, sf::Vector2f position) : Animatable(svc, "breakables", {16, 16}), m_collider{map, {32.f, 32.f}}, m_health{4.f} {
	get_collider().physics.position = position;
	get_collider().sync_components();
	m_collider.get()->set_attribute(shape::ColliderAttributes::fixed);
	m_collider.get()->set_trait(shape::ColliderTrait::block);
	m_collider.get()->set_trait(shape::ColliderTrait::breakable);
	get_collider().hurtbox.set_dimensions({38.f, 38.f});
	NANI_LOG_DEBUG(m_logger, "Breakable Position: {:.3f}, {:.3f}", get_collider().physics.position.x / 32.f, get_collider().physics.position.y / 32.f);
}

void Breakable::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	if (is_destroyed()) { return; }
	tick();
	energy = std::clamp(energy - dampen, 0.f, energy);
	m_flash_energy = std::clamp(m_flash_energy - dampen * 3.f, 0.f, m_flash_energy);
	if (energy < 0.2f) {
		energy = 0.f;
		m_flash_energy = 0.f;
		random_offset = {};
	} else {
		if (svc.ticker.every_x_ticks(20)) { random_offset = random::random_vector_float(-energy, energy); }
	}
	handle_collision(player.get_collider());
	set_channel(map.get_style_id());
	set_frame((m_health.get_i_quantity() - 1) + 4 * static_cast<int>(std::ceil(m_flash_energy / 4.f)));
}

void Breakable::handle_collision(shape::Collider& other) const {
	if (is_destroyed()) { return; }
	other.handle_collider_collision(*m_collider.get());
}

void Breakable::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (is_destroyed()) { return; }
	auto render_position = util::round_to(get_collider().physics.position, constants::f_cell_size);
	Animatable::set_position(render_position - cam + random_offset);
	if (!debug::is_production()) {
		get_collider().render(win, cam);
	} else {
		win.draw(*this);
		++debug::draw_calls;
	}
}

void Breakable::on_hit(automa::ServiceProvider& svc, Map& map, arms::Projectile& proj) {
	if (proj.transcendent()) { return; }
	if (is_destroyed()) { return; }
	if (proj.get_collider().collides_with(get_collider().hurtbox)) {
		if (!proj.destruction_initiated()) {
			on_smash(svc, map, proj.get_power());
			proj.on_explode(svc, map);
		}
		proj.destroy(false);
		map.spawn_effect(svc, "hit_flash", proj.get_position());
	}
}

void Breakable::on_smash(automa::ServiceProvider& svc, world::Map& map, float power) {
	if (is_destroyed()) { return; }
	auto diff = m_health.get_i_quantity();
	m_health.inflict(power);
	auto const hit_energy = 8.f;
	energy = hit_energy;
	m_flash_energy = hit_energy;
	svc.soundboard.flags.world.set(audio::World::breakable_hit);
	if (is_destroyed()) {
		map.spawn_effect(svc, "small_explosion", get_collider().get_center());
		map.spawn_emitter(svc, "breakable", get_collider().get_position(), Direction{}, {32.f, 32.f});
		svc.soundboard.flags.world.set(audio::World::breakable_shatter);
	}
}

} // namespace fornani::world
