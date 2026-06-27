
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>

#define LYMPHOCYTE_BIND(f) std::bind(&Lymphocyte::f, this)

namespace fornani::enemy {

enum class LymphocyteState { spawn, idle, make_antibody, turn, dormant };
enum class LymphocyteFlags { alerted };

class Lymphocyte final : public Enemy, public StateMachine<LymphocyteState> {
  public:
	Lymphocyte(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Lymphocyte::update_dormant, this);
	fsm::StateFunction update_dormant();
	fsm::StateFunction update_spawn();
	fsm::StateFunction update_idle();
	fsm::StateFunction update_make_antibody();
	fsm::StateFunction update_turn();

  private:
	bool change_state(LymphocyteState next, anim::Parameters params);
	void spawn_antibody(int amount);

  private:
	components::SteeringBehavior m_steering{};
	util::BitFlags<LymphocyteFlags> m_flags{};

	automa::ServiceProvider* m_services;
	world::Map* m_map;
};

} // namespace fornani::enemy
