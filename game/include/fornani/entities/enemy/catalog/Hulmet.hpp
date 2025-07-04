
#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/Attack.hpp"
#include "fornani/entities/packages/Caution.hpp"
#define HULMET_BIND(f) std::bind(&Hulmet::f, this)

namespace fornani::enemy {

enum class HulmetState { idle, turn, run, jump, alert, sleep, shoot, roll };
enum class HulmetVariant { gunner, bodyguard };

class Hulmet final : public Enemy {
  public:
	Hulmet(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;

	fsm::StateFunction state_function = std::bind(&Hulmet::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_alert();
	fsm::StateFunction update_sleep();
	fsm::StateFunction update_shoot();
	fsm::StateFunction update_roll();

	[[nodiscard]] auto is_mid_run() { return m_cooldowns.run.is_almost_complete(); }

  private:
	void request(HulmetState to) { m_state.desired = to; }

	struct {
		HulmetState actual{};
		HulmetState desired{};
	} m_state{};

	HulmetVariant m_variant{};

	// packages
	struct {
		entity::FloatingPart gun;
	} m_parts;

	entity::Caution m_caution{};
	entity::WeaponPackage m_weapon;

	struct {
		util::Cooldown post_fire{400};
		util::Cooldown alerted{1600};
		util::Cooldown post_jump{200};
		util::Cooldown post_roll{60};
		util::Cooldown run{80};
	} m_cooldowns{};

	// lookup, duration, framerate, num_loops
	struct {
		anim::Parameters idle{0, 6, 32, -1};
		anim::Parameters turn{26, 2, 32, 0};
		anim::Parameters run{5, 4, 28, 4};
		anim::Parameters jump{9, 1, 64, 0};
		anim::Parameters roll{10, 4, 24, 2};
		anim::Parameters sleep{14, 6, 64, 3};
		anim::Parameters alert{20, 6, 24, 0};
		anim::Parameters shoot{28, 3, 24, 1};
	} m_animations{};

	float m_jump_force{-34.f};
	int m_jump_time{4};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(HulmetState next, anim::Parameters params);
};

} // namespace fornani::enemy
