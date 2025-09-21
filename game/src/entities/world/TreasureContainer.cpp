
#include <fornani/entities/world/TreasureContainer.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::entity {

TreasureContainer::TreasureContainer(automa::ServiceProvider& svc, item::Rarity rarity, sf::Vector2f position, int index)
	: Animatable(svc, "treasure_ball", {16, 16}), rarity(rarity), index(index), m_neutral{0, 1, 1024, 0}, m_shine{1, 5, 24, 0} {
	gravitator = vfx::Gravitator(sf::Vector2f{}, sf::Color::Transparent, 0.8f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2f{0.8f, 0.8f}, 1.0f);
	gravitator.set_position(position);
	health.set_max(4.f);
	set_channel(static_cast<int>(rarity));
	center();
	sensor.bounds.setRadius(16.f);
	sensor.bounds.setOrigin({16.f, 16.f});
	loot_multiplier = 1.f + static_cast<float>(rarity) * 4.f;
	root = random::random_vector_float(-16.f, 16.f);
	set_parameters(m_neutral);
	random_frame_start();
}

void TreasureContainer::update(automa::ServiceProvider& svc, sf::Vector2f target) {
	tick();
	gravitator.set_target_position(target + root);
	gravitator.update(svc);
	sensor.set_position(gravitator.position());
	health.update();
	if (Animatable::is_complete()) {
		m_state = m_state == TreasureContainerState::neutral ? TreasureContainerState::shine : TreasureContainerState::neutral;
		switch (m_state) {
		case TreasureContainerState::neutral: set_parameters(m_neutral); break;
		case TreasureContainerState::shine: set_parameters(m_shine); break;
		}
	}
}

void TreasureContainer::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (health.is_dead()) { return; }
	if (sensor.within_bounds(proj.get_collider())) {
		if (!proj.destruction_initiated()) {
			health.inflict(proj.get_damage());
			svc.soundboard.flags.world.set(audio::World::breakable_hit);
			if (health.is_dead()) {
				map.active_loot.push_back(item::Loot(svc, {2, 3}, loot_multiplier, gravitator.position(), 0, rarity == item::Rarity::priceless, 0));
				svc.soundboard.flags.world.set(audio::World::block_toggle);
				map.effects.push_back(entity::Effect(svc, "small_explosion", sensor.bounds.getPosition() - sf::Vector2f{8.f, 8.f}));
			}
		}
		proj.destroy(false);
	}
}

void TreasureContainer::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (health.is_dead()) { return; }
	if (svc.greyblock_mode()) {
		sensor.render(win, cam);
	} else {
		Animatable::set_position(gravitator.position() - cam);
		win.draw(*this);
	}
}

} // namespace fornani::entity
