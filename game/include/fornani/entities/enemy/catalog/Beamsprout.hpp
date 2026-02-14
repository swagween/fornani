
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/particle/Spring.hpp>
#include <fornani/utils/Flaggable.hpp>
#define BEAMSPROUT_BIND(f) std::bind(&Beamsprout::f, this)

namespace fornani::enemy {

enum class BeamsproutState { idle, charge, shoot, relax, turn };
enum class BeamsproutFlags { spit };

class Beamsprout final : public Enemy, public Flaggable<BeamsproutFlags>, public StateMachine<BeamsproutState> {

  public:
	Beamsprout(automa::ServiceProvider& svc, world::Map& map, sf::Vector2<int> start_direction);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Beamsprout::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_charge();
	fsm::StateFunction update_shoot();
	fsm::StateFunction update_relax();
	fsm::StateFunction update_turn();

  private:
	void set_root(world::Map& map);

	// packages
	entity::WeaponPackage beam;

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	vfx::Spring* m_root{};
	components::SteeringBehavior m_steering{};

	util::Cooldown post_beam;
	util::Cooldown init{80};

	float fire_rate;

	bool change_state(BeamsproutState next, anim::Parameters params);
};

} // namespace fornani::enemy
