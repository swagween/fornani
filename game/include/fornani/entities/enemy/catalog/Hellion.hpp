
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#define HELLION_BIND(f) std::bind(&Hellion::f, this)

namespace fornani::enemy {

enum class HellionState : std::uint8_t { run, sleep, awaken, turn };

class Hellion final : public Enemy, public StateMachine<HellionState> {

  public:
	Hellion(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Hellion::update_sleep, this);
	fsm::StateFunction update_run();
	fsm::StateFunction update_sleep();
	fsm::StateFunction update_awaken();
	fsm::StateFunction update_turn();

  private:
	// packages
	entity::WeaponPackage m_poison;

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	sf::Vector2f m_target{};

	bool change_state(HellionState next, anim::Parameters params);
};

} // namespace fornani::enemy
