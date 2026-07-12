
#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/events/GameplayEvent.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::enemy {

Boss::Boss(automa::ServiceProvider& svc, world::Map& map, std::string_view label)
	: Enemy{svc, map, label}, Animatable(svc, "enemy_" + std::string{label}, sf::Vector2i{svc.data.enemy[label]["physical"]["sprite_dimensions"][0].as<int>(), svc.data.enemy[label]["physical"]["sprite_dimensions"][1].as<int>()}),
	  p_health_bar{svc, label}, p_services{&svc}, p_map{&map} {
	svc.events.start_battle_event.attach_to(slot, &Boss::start_battle, this);
	flags.general.set(GeneralFlags::boss);
}

void Boss::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	player.set_flag(player::PlayerFlags::boss_fight, battle_mode());
	has_flag_set(BossFlags::battle_mode) ? flags.state.reset(StateFlags::pre_battle_invincibility) : flags.state.set(StateFlags::pre_battle_invincibility);
	Enemy::update(svc, map, player);
	p_health_bar.update(health.get_normalized());
	if (health.is_dead() && !has_flag_set(BossFlags::end_battle)) { end_battle(); }

	// make sure boss stays in bounds;
	if (!map.within_bounds(get_collider().get_center())) { m_oob_counter.update(); }
	if (m_oob_counter.get_count() > 400) {
		m_oob_counter.cancel();
		Enemy::set_position(map.get_closest_home_point(get_collider().get_center()));
		map.spawn_effect(svc, "medium_flash", get_collider().get_center());
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
	p_services->soundboard.play_sound("boss_defeat");
	p_services->ticker.slow_down(128);
	p_services->camera_controller.shake(10, 0.4f, 900);
}

} // namespace fornani::enemy
