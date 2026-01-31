
#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/events/GameplayEvent.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::enemy {

Boss::Boss(automa::ServiceProvider& svc, world::Map& map, std::string_view label) : Enemy{svc, map, label}, p_health_bar{svc, label}, p_services{&svc} {
	svc.events.get_or_add<StartBattleEvent>().subscribe([this]() { this->start_battle(); });
	flags.general.set(GeneralFlags::boss);
}

void Boss::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	p_health_bar.update(health.get_normalized());
}

void Boss::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (has_flag_set(BossFlags::battle_mode)) { p_health_bar.render(win); }
}

} // namespace fornani::enemy
