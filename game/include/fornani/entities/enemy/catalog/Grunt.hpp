
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>

#define GRUNT_BIND(f) std::bind(&Grunt::f, this)

namespace fornani::enemy {

enum class GruntState { idle, drink };
enum class GruntVariant { typist };

class Grunt final : public Enemy, public StateMachine<GruntState> {
  public:
	Grunt(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Grunt::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_drink();

  private:
	GruntVariant m_variant;

	bool change_state(GruntState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
