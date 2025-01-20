#pragma once

#include "../Enemy.hpp"
#define BEAMSTALK_BIND(f) std::bind(&Beamstalk::f, this)

namespace enemy {

enum class BeamstalkState { idle, charge, shoot, relax };

class Beamstalk : public Enemy {

  public:
	Beamstalk() = delete;
	~Beamstalk() override {}
	Beamstalk& operator=(Beamstalk&&) = delete;
	Beamstalk(automa::ServiceProvider& svc, world::Map& map, sf::Vector2<int> start_direction);
	void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;

	fsm::StateFunction state_function = std::bind(&Beamstalk::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_charge();
	fsm::StateFunction update_shoot();
	fsm::StateFunction update_relax();

  private:
	BeamstalkState state{};

	// packages
	entity::WeaponPackage beam;

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{14, 14, 36, -1};
	anim::Parameters charge{0, 9, 36, 0};
	anim::Parameters shoot{9, 3, 36, 4};
	anim::Parameters relax{12, 2, 36, 0};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	util::Cooldown post_beam{128};

	float fire_rate{};

	bool change_state(BeamstalkState next, anim::Parameters params);
};

} // namespace enemy