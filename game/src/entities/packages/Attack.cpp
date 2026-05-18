
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::entity {

void Attack::update() {
	hit.bounds.setOrigin({hit.bounds.getRadius(), hit.bounds.getRadius()});
	sensor.bounds.setOrigin({sensor.bounds.getRadius(), sensor.bounds.getRadius()});
}

void Attack::set_position(sf::Vector2f position) {
	sensor.bounds.setPosition(position + origin);
	hit.bounds.setPosition(position + hit_offset + origin);
}

void Attack::handle_player(player::Player& player) { sensor.within_bounds(player.hurtbox) ? sensor.activate() : sensor.deactivate(); }

bool Attack::hurt_player(player::Player& player, float damage, sf::Vector2f knockback) {
	if (hit.within_bounds(player.hurtbox) && hit.active()) {
		if (!player.invincible() && !player.health.is_dead()) { player.accumulated_forces.push_back(knockback); }
		player.hurt(damage);
		return true;
	}
	return false;
}

bool Attack::kill_player(player::Player& player, player::PlayerDeathType death) {
	if (hit.within_bounds(player.hurtbox) && hit.active()) {
		player.set_death_type(death);
		player.hurt(max_damage_v);
		return true;
	}
	return false;
}

void Attack::cancel_projectiles(automa::ServiceProvider& svc, world::Map& map, arms::Team team, int freezeframe) {
	if (hit.active()) {
		for (auto& proj : map.active_projectiles) {
			if (proj.get_team() == team) { continue; }
			if (hit.within_bounds(proj.get_collider())) {
				proj.handle_hard_hit(svc, map);
				random::percent_chance(50) ? svc.soundboard.play_sound("projectile_ping_1", proj.get_collider().get_global_center()) : svc.soundboard.play_sound("projectile_ping_2", proj.get_collider().get_global_center());
				proj.destroy(false);
				svc.ticker.freeze_frame(freezeframe);
			}
		}
	}
}

void Attack::set_constant_radius(float to) {
	sensor.bounds.setRadius(to);
	hit.bounds.setRadius(to);
}

void Attack::render(sf::RenderWindow& win, sf::Vector2f cam) {
	// sensor.render(win, cam);
	hit.render(win, cam);
}

void Attack::enable() { hit.activate(); }

void Attack::disable() { hit.deactivate(); }

} // namespace fornani::entity
