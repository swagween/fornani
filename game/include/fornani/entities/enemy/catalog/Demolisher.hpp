#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/entities/packages/Shockwave.hpp>
#include <fornani/utils/Flaggable.hpp>

#define DEMOLISHER_BIND(f) std::bind(&Demolisher::f, this)

namespace fornani::enemy {

enum class DemolisherState : std::uint8_t { idle, begin_run, run, ground_pound, turn };
enum class DemolisherVariant : std::uint8_t { brute };
enum class DemolisherFlags : std::uint8_t { step_detected, projectile };

class Demolisher final : public Enemy, public StateMachine<DemolisherState>, public Flaggable<DemolisherFlags> {
  public:
	Demolisher(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	fsm::StateFunction state_function = std::bind(&Demolisher::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_begin_run();
	fsm::StateFunction update_run();
	fsm::StateFunction update_ground_pound();
	fsm::StateFunction update_turn();

  private:
	DemolisherVariant m_variant{};

	util::Cooldown m_jump_time;
	util::Cooldown m_switch_sides;
	entity::Attack m_attack{};
	entity::Attack m_helmet{};
	entity::Caution m_caution{};
	std::vector<entity::Shockwave> m_shockwaves{};

	bool change_state(DemolisherState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
