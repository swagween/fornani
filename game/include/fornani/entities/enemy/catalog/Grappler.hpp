
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Attack.hpp>
#define GRAPPLER_BIND(f) std::bind(&Grappler::f, this)

namespace fornani::enemy {

enum class GrapplerState : std::uint8_t { idle, leap, snag, release, whiff, turn };
enum class GrapplerFlags : std::uint8_t { caught_player, released_player };

class Grappler final : public Enemy, public StateMachine<GrapplerState> {

  public:
	Grappler(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Grappler::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_leap();
	fsm::StateFunction update_snag();
	fsm::StateFunction update_release();
	fsm::StateFunction update_whiff();
	fsm::StateFunction update_turn();

  private:
	automa::ServiceProvider* m_services;
	world::Map* m_map;

	util::Cooldown m_hold_time{};
	util::Cooldown m_leap_cooldown{};
	util::BitFlags<GrapplerFlags> m_flags{};
	entity::Attack m_grab{};

	bool change_state(GrapplerState next, anim::Parameters params);
};

} // namespace fornani::enemy
