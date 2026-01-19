
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/gui/BossHealth.hpp>

namespace fornani::enemy {

enum class BossFlags { battle_mode, second_phase, post_death };

class Boss : public Enemy, public Flaggable<BossFlags> {
  public:
	Boss(automa::ServiceProvider& svc, world::Map& map, std::string_view label) : Enemy{svc, map, label}, p_health_bar{svc, label}, p_services{&svc} {}
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

  protected:
	automa::ServiceProvider* p_services;
	gui::BossHealth p_health_bar;
};

} // namespace fornani::enemy
