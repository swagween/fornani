
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Caution.hpp>
#define HURTLE_BIND(f) std::bind(&Hurtle::f, this)

namespace fornani::enemy {

enum class HurtleState : std::uint8_t { run, sleep, turn };

class Hurtle final : public Enemy, public StateMachine<HurtleState> {

  public:
	Hurtle(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Hurtle::update_run, this);
	fsm::StateFunction update_run();
	fsm::StateFunction update_sleep();
	fsm::StateFunction update_turn();

  private:
	automa::ServiceProvider* m_services;
	world::Map* m_map;

	util::Cooldown m_turn{};
	entity::Caution m_caution{};

	bool change_state(HurtleState next, anim::Parameters params);
};

} // namespace fornani::enemy
