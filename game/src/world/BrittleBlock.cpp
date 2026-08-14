
#include <fornani/core/Debug.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/particle/Effect.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/BrittleBlock.hpp>
#include <fornani/world/Map.hpp>
#include <algorithm>

namespace fornani::world {

BrittleBlock::BrittleBlock(automa::ServiceProvider& svc, Map& map, sf::Vector2f position, int chunk_id)
	: Animatable(svc, "brittle_blocks", {18, 18}), m_chunk_id{chunk_id}, health{70.f}, hit_energy{2.99f}, m_map{&map}, m_collider{map, {32.f, 32.f}}, m_death{1600} {
	m_collider.get()->physics.position = position;
	push_and_set_animation("default", {0, 3, 24, -1});
	m_collider.get()->set_trait(shape::ColliderTrait::block);
	m_collider.get()->set_attribute(shape::ColliderAttributes::fixed);
	set_origin({1.f, 1.f});
}

void BrittleBlock::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	m_collider.get()->sync_components();
	m_collider.get()->set_attribute(shape::ColliderAttributes::no_collision, is_destroyed());
	if (m_death.running()) {
		m_death.update();
		if (m_death.is_almost_complete()) {
			m_death.cancel();
			svc.soundboard.play_sound("block_toggle");
			map.spawn_effect(svc, "small_explosion", m_collider.get()->get_center(), {}, 3);
			health.refill();
		}
		return;
	}
	if (is_destroyed()) {
		svc.ticker.freeze_frame(0.015f);
		svc.soundboard.play_sound("brittle_shatter");
		map.spawn_emitter(svc, "breakable", m_collider.get()->physics.position, Direction{UND::up}, m_collider.get()->dimensions);
		map.spawn_effect(svc, "small_explosion", m_collider.get()->get_center(), {}, 3);
		m_death.start();
		return;
	}
	tick();
	if (player.get_collider().jumpbox.overlaps(m_collider.get()->bounding_box)) { hit(); }
	energy = std::clamp(energy - dampen, 0.f, std::numeric_limits<float>::max());
	if (energy < 0.2f) { energy = 0.f; }
	if (svc.ticker.every_x_ticks(20)) { random_offset = random::random_vector_float(-energy, energy); }
	handle_collision(player.get_collider());
	set_frame(2 - static_cast<int>(health.get_normalized() * 2.f));
}

void BrittleBlock::handle_collision(shape::Collider& other) const {
	if (is_destroyed()) { return; }
	other.handle_collider_collision(*m_collider.get());
}

void BrittleBlock::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (is_destroyed()) { return; }
	set_position(m_collider.get()->physics.position - cam + random_offset);
	if (!debug::is_production()) {
		m_collider.get()->render(win, cam);
	} else {
		win.draw(*this);
		++debug::draw_calls;
	}
}

void BrittleBlock::on_hit(automa::ServiceProvider& svc, Map& map, arms::Projectile& proj, int power) {
	if (is_destroyed()) { return; }
	if (proj.transcendent()) { return; }
	if (proj.reflect()) {
		proj.get_collider().handle_collision(m_collider.get()->bounding_box);
		return;
	}
	if (proj.get_collider().collides_with(m_collider.get()->bounding_box)) { proj.handle_hard_hit(svc, map); }
}

void BrittleBlock::hit() {
	health.inflict(1.f);
	energy = hit_energy;
}

} // namespace fornani::world
