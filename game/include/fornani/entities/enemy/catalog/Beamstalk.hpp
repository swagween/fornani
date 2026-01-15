#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/utils/Flaggable.hpp>
#define BEAMSTALK_BIND(f) std::bind(&Beamstalk::f, this)

namespace fornani::enemy {

enum class BeamstalkState { idle, charge, shoot, relax };
enum class BeamstalkFlags { spit };

class Beamstalk final : public Enemy, public Flaggable<BeamstalkFlags>, public StateMachine<BeamstalkState> {

  public:
	Beamstalk(automa::ServiceProvider& svc, world::Map& map, sf::Vector2<int> start_direction);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Beamstalk::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_charge();
	fsm::StateFunction update_shoot();
	fsm::StateFunction update_relax();

  private:
	void set_root(world::Map& map);

	// packages
	entity::WeaponPackage beam;

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	sf::Vector2f* m_root{};
	components::SteeringBehavior m_steering{};

	util::Cooldown post_beam{128};
	util::Cooldown init{80};

	float fire_rate;

	bool change_state(BeamstalkState next, anim::Parameters params);
};

} // namespace fornani::enemy
