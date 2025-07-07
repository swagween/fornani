#include "fornani/world/Destroyable.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Destroyable::Destroyable(automa::ServiceProvider& svc, sf::Vector2<int> pos, int quest_id, int style_id) : position(pos), quest_id(quest_id), sprite{svc.assets.get_texture("destroyables")} {
	collider = shape::Collider(constants::f_cell_vec);
	auto f_pos = sf::Vector2f{static_cast<float>(position.x * constants::f_cell_size), static_cast<float>(position.y * constants::f_cell_size)};
	collider.physics.position = f_pos;
	collider.sync_components();
	sprite.setScale(constants::f_scale_vec);
	sprite.setTextureRect(sf::IntRect{{style_id * constants::i_cell_size, 0}, constants::i_cell_vec});
}

void Destroyable::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	if (detonated()) { return; }
	if (svc.quest.get_progression(fornani::QuestType::destroyers, quest_id) > 0) { flags.set(DestroyerState::detonated); }
	if (flags.test(DestroyerState::detonated)) {
		svc.data.destroy_block(quest_id);
		map.effects.push_back(entity::Effect(svc, "small_explosion", static_cast<sf::Vector2f>(position) * constants::f_cell_size, {}, 0, 0));
		svc.soundboard.flags.world.set(audio::World::block_toggle);
	}
	player.collider.handle_collider_collision(collider);
	for (auto& e : map.enemy_catalog.enemies) { e->get_collider().handle_collider_collision(collider); }
	for (auto& c : map.chests) { c.get_collider().handle_collider_collision(collider); }
}

void Destroyable::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
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
			map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_destruction_point() + proj.get_position(), {}, proj.effect_type(), 2));
			if (proj.get_direction().lnr == LNR::neutral) { map.effects.back().rotate(); }
			svc.soundboard.flags.world.set(audio::World::wall_hit);
		}
		proj.destroy(false);
	}
}

shape::Shape& Destroyable::get_bounding_box() { return collider.bounding_box; }

} // namespace fornani::world
