
#pragma once

#include <fornani/audio/Soundboard.hpp>
#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/utils/Flaggable.hpp>
#define SENTINEL_BIND(f) std::bind(&Sentinel::f, this)

namespace fornani::enemy {

enum class SentinelState { idle, run, turn, jump, land, swipe, slash, charge_swipe, charge_slash, prepare_dash, dash, summon };
enum class SentinelFlags { show_weapon, shorthop };
enum class SentinelVariant { knight, duelist };
enum class SentinelMode { neutral, hostile };

class Sentinel final : public Enemy, public StateMachine<SentinelState>, public Flaggable<SentinelFlags> {

  public:
	Sentinel(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Sentinel::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_run();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_land();
	fsm::StateFunction update_swipe();
	fsm::StateFunction update_slash();
	fsm::StateFunction update_charge_swipe();
	fsm::StateFunction update_charge_slash();
	fsm::StateFunction update_prepare_dash();
	fsm::StateFunction update_dash();
	fsm::StateFunction update_summon();

	[[nodiscard]] auto is_mid_run() { return m_cooldowns.run.is_almost_complete(); }
	[[nodiscard]] auto has_been_alerted() { return m_cooldowns.alerted.running(); }

  private:
	void jump(bool forward);
	bool change_state(SentinelState next, anim::Parameters params);
	void debug();

  private:
	SentinelVariant m_variant{};
	SentinelMode m_mode{};

	struct {
		util::Cooldown alerted{2000};
		util::Cooldown post_jump{200};
		util::Cooldown run{80};
		util::Cooldown post_attack{480};
	} m_cooldowns{};

	float fire_chance{50.f};

	// packages
	std::optional<entity::FloatingPart> m_shield;
	entity::FloatingPart m_sword;
	entity::Caution m_caution{};
	std::array<entity::Attack, 3> m_attacks{};

	// extra detectors
	shape::Shape m_vertical_range{};
	shape::Shape m_shoulders{};
	shape::Shape m_lower_range{};

	automa::ServiceProvider* m_services;
	world::Map* m_map;
};

} // namespace fornani::enemy
