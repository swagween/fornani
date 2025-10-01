
#include "fornani/world/Destructible.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Destructible::Destructible(automa::ServiceProvider& svc, dj::Json const& in, int style_id)
	: IWorldPositionable({in["position"][0].as<std::uint32_t>(), in["position"][1].as<std::uint32_t>()}), quest_id(in["id"].as<int>()), sprite{svc.assets.get_texture("destructibles")} {
	collider = shape::Collider(constants::f_cell_vec);
	collider.physics.position = get_world_position();
	collider.sync_components();
	sprite.setScale(constants::f_scale_vec);
	sprite.setTextureRect(sf::IntRect{{style_id * constants::i_cell_resolution, 0}, constants::i_resolution_vec});
	in["inverse"].as_bool() ? m_attributes.set(DestructibleAttributes::inverse) : m_attributes.reset(DestructibleAttributes::inverse);
	in["enemy_clear"].as_bool() ? m_attributes.set(DestructibleAttributes::enemy_clear) : m_attributes.reset(DestructibleAttributes::enemy_clear);
	m_state = in["inverse"].as_bool() ? 1 : 0;
	auto state = svc.data.get_destructible_state(quest_id);
	if (state != -1) { m_state = state; }
}

auto Destructible::ignore_updates() const -> bool { return is_destroyed(); }

void Destructible::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	if (map.enemies_cleared() && is_enemy_clear() && is_solid()) { svc.data.switch_destructible_state(quest_id, is_inverse()); }
	auto state = svc.data.get_destructible_state(quest_id);
	if (state != m_state && state != -1) {
		m_state = state;
		map.effects.push_back(entity::Effect(svc, "small_explosion", get_global_center()));
		svc.soundboard.flags.world.set(audio::World::block_toggle);
	}
	if (ignore_updates()) { return; }
	player.collider.handle_collider_collision(collider);
	for (auto& e : map.enemy_catalog.enemies) { e->get_collider().handle_collider_collision(collider); }
	for (auto& c : map.chests) { c.get_collider().handle_collider_collision(collider); }
}

void Destructible::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (ignore_updates()) { return; }
	sprite.setPosition(collider.physics.position - cam);
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
	} else {
		win.draw(sprite);
	}
}

void Destructible::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (ignore_updates()) { return; }
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

shape::Shape& Destructible::get_bounding_box() { return collider.bounding_box; }

} // namespace fornani::world
