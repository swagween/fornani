
#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/events/GameplayEvent.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::enemy {

Boss::Boss(automa::ServiceProvider& svc, world::Map& map, std::string_view label) : Enemy{svc, map, label}, p_health_bar{svc, label}, p_services{&svc}, p_map{&map} {
	svc.events.start_battle_event.attach_to(slot, &Boss::start_battle, this);
	flags.general.set(GeneralFlags::boss);
}

void Boss::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	player.set_flag(player::PlayerFlags::boss_fight, battle_mode());
	has_flag_set(BossFlags::battle_mode) ? flags.state.reset(StateFlags::pre_battle_invincibility) : flags.state.set(StateFlags::pre_battle_invincibility);
	Enemy::update(svc, map, player);
	p_health_bar.update(health.get_normalized());
	if (health.is_dead() && !has_flag_set(BossFlags::end_battle)) { end_battle(); }

	auto const left = player.get_collider().get_center().x < get_collider().get_center().x && directions.actual.left();
	auto const right = player.get_collider().get_center().x > get_collider().get_center().x && directions.actual.right();
	set_flag(BossFlags::facing_player, left || right);

	// make sure boss stays in bounds;
	if (!map.within_bounds(get_collider().get_center()) && map.home_points.size() > 0 && !has_flag_set(BossFlags::ignore_oob)) { m_oob_counter.update(); }
	if (m_oob_counter.get_count() > 400) {
		m_oob_counter.cancel();
		Enemy::set_position(map.get_closest_home_point(get_collider().get_center()));
		map.spawn_effect(svc, "medium_flash", get_collider().get_center());
	}

	if (player.is_dead() && !has_flag_set(BossFlags::defeated_player) && battle_mode()) {
		svc.data.increment_boss_victory(label);
		auto const num = svc.data.get_number_of_boss_victories(label);
		if (!debug::is_production()) {
			auto const ending = num == 1 ? "st" : num == 2 ? "nd" : num == 3 ? "rd" : "th";
			svc.notifications.push_notification(svc, "Defeated by " + label + " for the " + std::to_string(num) + ending + " time.");
		}
		set_flag(BossFlags::defeated_player);
	}
}

void Boss::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { p_health_bar.render(win); }

void Boss::start_battle() {
	flags.state.set(StateFlags::vulnerable);
	p_health_bar.bring_in();
	set_flag(BossFlags::start_battle);
	set_flag(BossFlags::battle_mode);
}

void Boss::end_battle() {
	set_flag(BossFlags::end_battle);
	set_flag(BossFlags::battle_mode, false);
	p_health_bar.send_out();
	// p_map->spawn_frame_effect(*p_services, "impact_spark", get_collider().get_center(), {}, 0);
	p_map->spawn_frame_effect(*p_services, "impact_frame_2", get_collider().get_center(), {}, 0);
	p_services->soundboard.play_sound("boss_defeat");
	p_services->soundboard.play_sound("boss_death");
	p_services->ticker.freeze_frame(1.2f, 0.6f);
	p_services->camera_controller.shake(4, 0.1f, 2000, 95);
}

bool Boss::is_staggered() {
	if (p_stagger_points.empty()) { return false; }

	auto const& p = p_stagger_points.back();
	if (health.get_normalized() >= p) { return false; }

	p_stagger_points.pop_back();
	return true;
}

auto Boss::is_facing_player() -> bool { return has_flag_set(BossFlags::facing_player); }

} // namespace fornani::enemy
