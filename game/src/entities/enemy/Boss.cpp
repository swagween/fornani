
#include <fornani/entities/enemy/Boss.hpp>

namespace fornani::enemy {

void Boss::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (has_flag_set(BossFlags::battle_mode)) { p_health_bar.render(win); }
}

} // namespace fornani::enemy
