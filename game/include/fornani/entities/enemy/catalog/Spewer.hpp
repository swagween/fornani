
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/WeaponPackage.hpp>

#define SPEWER_BIND(f) std::bind(&Spewer::f, this)

namespace fornani::enemy {

enum class SpewerState : std::uint8_t { idle, walk, prepare, spew, turn };

class Spewer final : public Enemy, public StateMachine<SpewerState> {
  public:
	Spewer(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Spewer::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_walk();
	fsm::StateFunction update_prepare();
	fsm::StateFunction update_spew();
	fsm::StateFunction update_turn();

  private:
	bool change_state(SpewerState next, anim::Parameters params);

	entity::WeaponPackage m_spew;

	world::Map* m_map;
	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
