#include "TreasureContainer.hpp"
#include "../../service/ServiceProvider.hpp"

namespace entity {

TreasureContainer::TreasureContainer(automa::ServiceProvider& svc, item::Rarity rarity, sf::Vector2<float> position, int index) : rarity(rarity), index(index) {
	gravitator = vfx::Gravitator(sf::Vector2<float>{}, sf::Color::Transparent, 0.8f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.8f, 0.8f}, 1.0f);
	gravitator.set_position(position);
	health.set_max(4.f);
	sprite.setTexture(svc.assets.t_treasure_ball);
	sprite.setTextureRect(sf::IntRect({0, static_cast<int>(rarity) * 16}, {16, 16}));
	sprite.setOrigin({8.f, 8.f});
	sensor.bounds.setRadius(8.f);
	sensor.bounds.setOrigin({8.f, 8.f});
}

void TreasureContainer::update(automa::ServiceProvider& svc, sf::Vector2<float> target) {
	gravitator.set_target_position(target);
	gravitator.update(svc);
	sensor.set_position(gravitator.position());
	health.update();
}

void TreasureContainer::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (health.is_dead()) { return; }
	if (proj.stats.transcendent) { return; }
	if (sensor.within_bounds(proj.bounding_box)) {
		if (!proj.destruction_initiated()) {
			health.inflict(proj.get_damage());
			svc.soundboard.flags.world.set(audio::World::breakable_hit);
			if (health.is_dead()) {
				map.active_loot.push_back(item::Loot(svc, {2, 3}, 1.f, gravitator.position(), 0, rarity == item::Rarity::priceless, 0));
				svc.soundboard.flags.world.set(audio::World::block_toggle);
				map.effects.push_back(entity::Effect(svc, sensor.bounds.getPosition() - sf::Vector2<float>{8.f, 8.f}, {}, 0, 0));
			}
		}
		proj.destroy(false);
	}
}

void TreasureContainer::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (health.is_dead()) { return; }
	if (svc.greyblock_mode()) {
		sensor.render(win, cam);
	} else {
		sprite.setPosition(gravitator.position() - cam);
		win.draw(sprite);
	}
}

} // namespace entity
