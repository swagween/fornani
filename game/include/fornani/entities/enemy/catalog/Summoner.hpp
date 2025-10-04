
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/entities/packages/Shockwave.hpp>
#include <fornani/gui/BossHealth.hpp>
#include <fornani/particle/Sparkler.hpp>

#define SUMMONER_BIND(f) std::bind(&Summoner::f, this)

namespace fornani::enemy {

enum class SummonerState : std::uint8_t { idle, walk, summon, horizontal_pulse, vertical_pulse, turn, begin_summon, dodge };
enum class SummonerVariant : std::uint8_t { mage, mother };

class Summoner final : public Enemy {
  public:
	Summoner(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	[[nodiscard]] auto invincible() const -> bool { return !flags.state.test(StateFlags::vulnerable); }
	[[nodiscard]] auto half_health() const -> bool { return health.get_hp() < health.get_max() * 0.5f; }
	[[nodiscard]] auto quarter_health() const -> bool { return health.get_hp() < health.get_max() * 0.25f; }
	[[nodiscard]] auto is_state(SummonerState test) const -> bool { return m_state.actual == test; }
	[[nodiscard]] auto is_pulsing() const -> bool { return is_state(SummonerState::horizontal_pulse) || is_state(SummonerState::vertical_pulse); }
	[[nodiscard]] auto is_summoning() const -> bool { return is_state(SummonerState::begin_summon) || is_state(SummonerState::summon); }

	fsm::StateFunction state_function = std::bind(&Summoner::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_walk();
	fsm::StateFunction update_begin_summon();
	fsm::StateFunction update_summon();
	fsm::StateFunction update_horizontal_pulse();
	fsm::StateFunction update_vertical_pulse();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_dodge();

  private:
	struct {
		SummonerState actual{};
		SummonerState desired{};
	} m_state{};

	struct {
		util::Cooldown post_summon;
		util::Cooldown walk;
		util::Cooldown post_walk;
		util::Cooldown post_hurt;
		util::Cooldown pulse;
	} m_cooldowns{};

	struct {
		util::Counter summon;
	} m_counters{};

	struct {
		std::array<entity::Attack, 3> pulse{};
	} m_attacks{};

	SummonerVariant m_variant{};
	void request(SummonerState to) { m_state.desired = to; }
	bool change_state(SummonerState next, anim::Parameters params);

	vfx::Sparkler m_magic;
	shape::Shape m_distant_range{};

	entity::Caution m_caution{};

	entity::WeaponPackage m_pulse;
	sf::Vector2f m_player_position{};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	sf::Vector2f m_home{};
};

} // namespace fornani::enemy
