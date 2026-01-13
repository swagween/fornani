
#include <ccmath/ext/clamp.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/particle/Effect.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Breakable.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::world {

Breakable::Breakable(automa::ServiceProvider& svc, Map& map, sf::Vector2f position) : Animatable(svc, "breakables", {16, 16}), m_collider{map, {32.f, 32.f}}, m_health{4.f} {
	get_collider().physics.position = position;
	get_collider().sync_components();
	m_collider.get()->set_attribute(shape::ColliderAttributes::fixed);
	m_collider.get()->set_trait(shape::ColliderTrait::block);
}

void Breakable::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	if (is_destroyed()) { return; }
	tick();
	energy = ccm::ext::clamp(energy - dampen, 0.f, std::numeric_limits<float>::max());
	if (energy < 0.2f) { energy = 0.f; }
	if (svc.ticker.every_x_ticks(20)) { random_offset = random::random_vector_float(-energy, energy); }
	handle_collision(player.get_collider());
	set_channel(map.get_style_id());
	set_frame(m_health.get_i_hp() - 1);
}

void Breakable::handle_collision(shape::Collider& other) const {
	if (is_destroyed()) { return; }
	other.handle_collider_collision(*m_collider.get());
}

void Breakable::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (is_destroyed()) { return; }
	Animatable::set_position(get_collider().physics.position - cam + random_offset);
	if (svc.greyblock_mode()) {
		get_collider().render(win, cam);
	} else {
		win.draw(*this);
	}
}

void Breakable::on_hit(automa::ServiceProvider& svc, Map& map, arms::Projectile& proj, int power) {
	if (proj.transcendent()) { return; }
	if (is_destroyed()) { return; }
	if (proj.get_collider().collides_with(get_collider().bounding_box)) {
		if (!proj.destruction_initiated()) {
			m_health.inflict(power == 1 ? static_cast<int>(proj.get_power()) : power);
			energy = hit_energy;
			svc.soundboard.flags.world.set(audio::World::breakable_hit);
		}
		if (is_destroyed()) {
			map.effects.push_back(entity::Effect(svc, "small_explosion", get_collider().get_center()));
			svc.soundboard.flags.world.set(audio::World::breakable_shatter);
		}
		proj.on_explode(svc, map);
		proj.destroy(false);
	}
}

void Breakable::on_smash(automa::ServiceProvider& svc, world::Map& map, int power) {
	if (is_destroyed()) { return; }
	m_health.inflict(power);
	energy = hit_energy;
	svc.soundboard.flags.world.set(audio::World::breakable_hit);
	if (is_destroyed()) {
		map.effects.push_back(entity::Effect(svc, "small_explosion", get_collider().physics.position));
		svc.soundboard.flags.world.set(audio::World::breakable_shatter);
	}
}

} // namespace fornani::world
