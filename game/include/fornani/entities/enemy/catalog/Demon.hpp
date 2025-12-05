#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/Attack.hpp"
#include "fornani/entities/packages/Caution.hpp"
#include "fornani/entities/packages/FloatingPart.hpp"
#define DEMON_BIND(f) std::bind(&Demon::f, this)

namespace fornani::enemy {

enum class DemonState { idle, turn, run, jump, signal, rush, stab, dormant, jumpsquat, uppercut };
enum class DemonFlags { player_behind, parrying };
enum class DemonVariant { warrior, spearman, duelist };

class Demon final : public Enemy {

  public:
	Demon(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	[[nodiscard]] auto is_dormant() const -> bool { return is_state(DemonState::dormant) || cooldowns.awaken.running(); }
	[[nodiscard]] auto is_state(DemonState test) const -> bool { return m_state.actual == test; }

	fsm::StateFunction state_function = std::bind(&Demon::update_dormant, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_signal();
	fsm::StateFunction update_rush();
	fsm::StateFunction update_stab();
	fsm::StateFunction update_uppercut();
	fsm::StateFunction update_dormant();
	fsm::StateFunction update_jumpsquat();

  private:
	DemonVariant m_variant{};

	util::BitFlags<DemonFlags> m_flags{};

	struct {
		DemonState actual{};
		DemonState desired{};
	} m_state{};

	// packages
	struct {
		entity::FloatingPart spear;
		entity::FloatingPart sword;
		entity::FloatingPart shield;
	} parts;

	struct {
		entity::Attack stab{};
	} attacks{};
	entity::Caution caution{};

	struct {
		util::Cooldown jump{40};
		util::Cooldown post_jump{400};
		util::Cooldown rush_hit{600};
		util::Cooldown post_rush{128};
		util::Cooldown awaken{180};
		util::Cooldown stab{6};
	} cooldowns{};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	float rand_jump{};

	void request(DemonState to) { m_state.desired = to; }
	bool change_state(DemonState next, anim::Parameters params);
};

} // namespace fornani::enemy
