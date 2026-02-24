
#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/events/GameplayEvent.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::enemy {

Boss::Boss(automa::ServiceProvider& svc, world::Map& map, std::string_view label) : Enemy{svc, map, label}, p_health_bar{svc, label}, p_services{&svc} {
	svc.events.start_battle_event.attach_to(slot, &Boss::start_battle, this);
	flags.general.set(GeneralFlags::boss);
}

void Boss::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	has_flag_set(BossFlags::battle_mode) ? flags.state.reset(StateFlags::pre_battle_invincibility) : flags.state.set(StateFlags::pre_battle_invincibility);
	Enemy::update(svc, map, player);
	p_health_bar.update(health.get_normalized());
	if (health.is_dead() && !has_flag_set(BossFlags::end_battle)) { end_battle(); }
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
	p_services->ticker.freeze_frame(64);
}

} // namespace fornani::enemy
