
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>

#define ANTIBODY_BIND(f) std::bind(&Antibody::f, this)

namespace fornani::enemy {

enum class AntibodyState { idle, stun };
enum class AntibodyVariant { igg, igm };

class Antibody final : public Enemy, public StateMachine<AntibodyState> {
  public:
	Antibody(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Antibody::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_stun();

  private:
	components::SteeringBehavior m_steering{};
	util::Cooldown m_post_stun{};
	AntibodyVariant m_variant;

	bool change_state(AntibodyState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
