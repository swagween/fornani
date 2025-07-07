#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/Attack.hpp"
#include "fornani/entities/packages/Caution.hpp"
#include "fornani/entities/packages/FloatingPart.hpp"
#define DEMON_BIND(f) std::bind(&Demon::f, this)

namespace fornani::enemy {

enum class DemonState { idle, turn, run, jump, signal, rush, stab, dormant, jumpsquat };
enum class DemonVariant { warrior, spearman };

class Demon final : public Enemy {

  public:
	Demon() = delete;
	~Demon() override {}
	Demon& operator=(Demon&&) = delete;
	Demon(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	[[nodiscard]] auto is_dormant() const -> bool { return state == DemonState::dormant || cooldowns.awaken.running(); }

	fsm::StateFunction state_function = std::bind(&Demon::update_dormant, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_signal();
	fsm::StateFunction update_rush();
	fsm::StateFunction update_stab();
	fsm::StateFunction update_dormant();
	fsm::StateFunction update_jumpsquat();

  private:
	DemonState state{};
	DemonVariant variant{};

	// packages
	struct {
		entity::FloatingPart spear;
		entity::FloatingPart sword;
		entity::FloatingPart shield;
	} parts;

	struct {
		entity::Attack stab{};
		entity::Attack rush{};
	} attacks{};
	entity::Caution caution{};


	struct {
		util::Cooldown jump{40};
		util::Cooldown post_jump{400};
		util::Cooldown rush_hit{600};
		util::Cooldown post_rush{128};
		util::Cooldown awaken{180};
	} cooldowns{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 6, 28, -1};
	anim::Parameters turn{9, 1, 48, 1};
	anim::Parameters run{6, 4, 28, 4};
	anim::Parameters jump{9, 1, 48, 0};
	anim::Parameters signal{10, 1, 58, 2};
	anim::Parameters rush{11, 1, 22, 3};
	anim::Parameters stab{11, 1, 32, 0};
	anim::Parameters jumpsquat{12, 1, 44, 1};
	anim::Parameters dormant{13, 1, 32, -1};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	float rand_jump{};

	bool change_state(DemonState next, anim::Parameters params);
};

} // namespace fornani::enemy
