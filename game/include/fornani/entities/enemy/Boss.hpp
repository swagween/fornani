
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/gui/BossHealth.hpp>

namespace fornani::enemy {

enum class BossFlags { start_battle, battle_mode, second_phase, post_death };

class Boss : public Enemy, public Flaggable<BossFlags> {
  public:
	Boss(automa::ServiceProvider& svc, world::Map& map, std::string_view label);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void start_battle() { set_flag(BossFlags::start_battle); }

  protected:
	automa::ServiceProvider* p_services;
	gui::BossHealth p_health_bar;
};

} // namespace fornani::enemy
