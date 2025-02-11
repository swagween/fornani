#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/Attack.hpp"
#include "fornani/entities/packages/Caution.hpp"
#include "fornani/entities/packages/FloatingPart.hpp"
#define IMP_BIND(f) std::bind(&Imp::f, this)

namespace fornani::enemy {

enum class ImpState { idle, turn, run, jump, fall, attack, dormant };
enum class ImpVariant { fork, knife };

class Imp final : public Enemy {

  public:
	Imp() = delete;
	~Imp() override {}
	Imp& operator=(Imp&&) = delete;
	Imp(automa::ServiceProvider& svc, world::Map& map);
	void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;
	[[nodiscard]] auto is_dormant() const -> bool { return state == ImpState::dormant || cooldowns.awaken.running(); }

	fsm::StateFunction state_function = std::bind(&Imp::update_dormant, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_fall();
	fsm::StateFunction update_attack();
	fsm::StateFunction update_dormant();

  private:
	ImpState state{};
	ImpVariant variant{};

	// packages
	struct {
		entity::FloatingPart knife;
		entity::FloatingPart fork;
		entity::FloatingPart hand;
	} parts;

	struct {
		entity::Attack stab{};
	} attacks{};
	entity::Caution caution{};

	struct {
		util::Cooldown jump{40};
		util::Cooldown post_jump{400};
		util::Cooldown rush_hit{600};
		util::Cooldown post_attack{128};
		util::Cooldown awaken{180};
	} cooldowns{};

	// lookup, duration, framerate, num_loops
	anim::Parameters dormant{0, 1, 32, -1};
	anim::Parameters idle{1, 6, 28, -1};
	anim::Parameters turn{7, 3, 48, 1};
	anim::Parameters run{10, 8, 28, -1};
	anim::Parameters jump{18, 5, 48, 0};
	anim::Parameters fall{24, 3, 58, -1};
	anim::Parameters attack{27, 7, 22, 0};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	float rand_jump{};

	bool change_state(ImpState next, anim::Parameters params);
};

} // namespace fornani::enemy
