
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/entities/enemy/Enemy.hpp>

#define CROW_BIND(f) std::bind(&Crow::f, this)

namespace fornani::enemy {

enum class CrowState { idle, peck, fly, turn };

class Crow final : public Enemy, public StateMachine<CrowState> {
  public:
	Crow(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f spread);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Crow::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_peck();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_fly();

  private:
	components::SteeringBehavior m_steering{};
	bool change_state(CrowState next, anim::Parameters params);
	sf::Vector2f m_home{};

	sf::Vector2f m_friction{};
	float m_home_force{};
	float m_evade_force{};

	automa::ServiceProvider* m_services;

	void debug();
};

} // namespace fornani::enemy
