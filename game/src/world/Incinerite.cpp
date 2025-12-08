
#include <ccmath/ext/clamp.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/particle/Effect.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Incinerite.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::world {

Incinerite::Incinerite(automa::ServiceProvider& svc, sf::Vector2f position, int chunk_id) : Animatable(svc, "incinerite", {16, 16}), m_chunk_id{chunk_id}, health{40.f}, hit_energy{2.99f} {
	collider = shape::Collider({32.f, 32.f});
	collider.physics.position = position;
	set_parameters({0, 3, 24, -1});
	push_animation("default", {0, 1, 24, -1});
	push_animation("shine", {1, 6, 24, 0});
}

void Incinerite::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	collider.sync_components();
	if (is_destroyed()) {
		svc.ticker.freeze_frame(2);
		svc.soundboard.flags.world.set(audio::World::incinerite_explosion);
		map.active_emitters.push_back(vfx::Emitter(svc, collider.physics.position, collider.dimensions, "incinerite", colors::nani_white));
		map.spawn_effect(svc, "small_explosion", collider.get_center(), {}, 3);
		return;
	}
	tick();
	if (svc.ticker.every_x_ticks(1000)) { set_animation("shine"); }
	if (animation.complete()) { set_animation("default"); }
	energy = ccm::ext::clamp(energy - dampen, 0.f, std::numeric_limits<float>::max());
	if (energy < 0.2f) { energy = 0.f; }
	if (svc.ticker.every_x_ticks(20)) { random_offset = random::random_vector_float(-energy, energy); }
	handle_collision(player.collider);
	set_channel(3 - static_cast<int>(health.get_normalized() * 3.f));
}

void Incinerite::handle_collision(shape::Collider& other) const {
	if (is_destroyed()) { return; }
	other.handle_collider_collision(collider);
}

void Incinerite::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (is_destroyed()) { return; }
	set_position(collider.physics.position - cam + random_offset);
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
	} else {
		win.draw(*this);
	}
}

void Incinerite::on_hit(automa::ServiceProvider& svc, Map& map, arms::Projectile& proj, int power) {
	if (is_destroyed()) { return; }
	if (proj.transcendent()) { return; }
	if (proj.reflect()) {
		proj.get_collider().handle_collision(collider.bounding_box);
		return;
	}
	if (proj.get_collider().collides_with(collider.bounding_box)) {
		if (!proj.destruction_initiated()) {
			map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_destruction_point() + proj.get_position()));
			if (proj.get_direction().lnr == LNR::neutral) { map.effects.back().rotate(); }
			svc.soundboard.flags.world.set(audio::World::hard_hit);
		}
		proj.destroy(false);
	}
}

void Incinerite::hit() {
	health.inflict(1.f);
	energy = hit_energy;
}

} // namespace fornani::world
