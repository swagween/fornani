#include "fornani/level/Destroyable.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/entities/player/Player.hpp"

namespace fornani::world {

Destroyable::Destroyable(automa::ServiceProvider& svc, sf::Vector2<int> pos, int quest_id, int style_id) : position(pos), quest_id(quest_id), sprite{svc.assets.tilesets.at(style_id)} {
	collider = shape::Collider({32.f, 32.f});
	auto f_pos = sf::Vector2<float>{static_cast<float>(position.x * svc.constants.cell_size), static_cast<float>(position.y * svc.constants.cell_size)};
	collider.physics.position = f_pos;
	collider.sync_components();
	sprite.setTextureRect(sf::IntRect{{6 * 32, 14 * 32}, {32, 32}});
}

void Destroyable::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	if (detonated()) { return; }
	if (svc.quest.get_progression(fornani::QuestType::destroyers, quest_id) > 0) { flags.set(DestroyerState::detonated); }
	if (flags.test(DestroyerState::detonated)) {
		svc.data.destroy_block(quest_id);
		map.effects.push_back(entity::Effect(svc, static_cast<sf::Vector2<float>>(position) * svc.constants.cell_size, {}, 0, 0));
		svc.soundboard.flags.world.set(audio::World::block_toggle);
	}
	player.collider.handle_collider_collision(collider);
	for (auto& e : map.enemy_catalog.enemies) { e->get_collider().handle_collider_collision(collider); }
	for (auto& c : map.chests) { c.get_collider().handle_collider_collision(collider); }
}

void Destroyable::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (detonated()) { return; }
	sprite.setPosition(collider.physics.position - cam);
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
	} else {
		win.draw(sprite);
	}
}

void Destroyable::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) const {
	if (detonated()) { return; }
	if (proj.transcendent()) { return; }
	if (proj.get_bounding_box().overlaps(collider.bounding_box)) {
		if (!proj.destruction_initiated()) {
			map.effects.push_back(entity::Effect(svc, proj.get_destruction_point() + proj.get_position(), {}, proj.effect_type(), 2));
			if (proj.get_direction().lr == dir::LR::neutral) { map.effects.back().rotate(); }
			svc.soundboard.flags.world.set(audio::World::wall_hit);
		}
		proj.destroy(false);
	}
}

shape::Shape& Destroyable::get_bounding_box() { return collider.bounding_box; }

} // namespace world
