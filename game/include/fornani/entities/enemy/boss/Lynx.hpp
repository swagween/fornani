
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/npc/NPC.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/entities/packages/Shockwave.hpp>
#include <fornani/gui/BossHealth.hpp>
#include <fornani/particle/Sparkler.hpp>

#define LYNX_BIND(f) std::bind(&Lynx::f, this)

namespace fornani::enemy {

enum class LynxState : std::uint8_t {
	sit,
	get_up,
	idle,
	jump,
	forward_slash,
	levitate,
	run,
	downward_slam,
	prepare_shuriken,
	toss_shuriken,
	upward_slash,
	triple_slash,
	turn,
	aerial_slash,
	prepare_slash,
	defeat,
	second_phase,
	laugh,
	stagger
};
enum class LynxFlags : std::uint8_t { conversing, battle_mode, second_phase, just_levitated, player_defeated };

class Lynx final : public Enemy, public npc::NPC {
  public:
	Lynx(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	[[nodiscard]] auto invincible() const -> bool { return !flags.state.test(StateFlags::vulnerable); }
	[[nodiscard]] auto half_health() const -> bool { return health.get_hp() < health.get_max() * 0.5f; }
	[[nodiscard]] auto quarter_health() const -> bool { return health.get_hp() < health.get_max() * 0.25f; }
	[[nodiscard]] auto is_state(LynxState test) const -> bool { return m_state.actual == test; }
	[[nodiscard]] auto is_levitating() const -> bool { return is_state(LynxState::levitate) || is_state(LynxState::second_phase); }
	[[nodiscard]] auto slam_follow() const -> bool { return is_state(LynxState::downward_slam) && half_health() && Enemy::animation.get_frame_count() < 6; }

	fsm::StateFunction state_function = std::bind(&Lynx::update_sit, this);
	fsm::StateFunction update_sit();
	fsm::StateFunction update_get_up();
	fsm::StateFunction update_idle();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_forward_slash();
	fsm::StateFunction update_levitate();
	fsm::StateFunction update_run();
	fsm::StateFunction update_downward_slam();
	fsm::StateFunction update_prepare_shuriken();
	fsm::StateFunction update_toss_shuriken();
	fsm::StateFunction update_upward_slash();
	fsm::StateFunction update_triple_slash();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_aerial_slash();
	fsm::StateFunction update_prepare_slash();
	fsm::StateFunction update_defeat();
	fsm::StateFunction update_second_phase();
	fsm::StateFunction update_laugh();
	fsm::StateFunction update_stagger();

  private:
	struct {
		LynxState actual{};
		LynxState desired{};
	} m_state{};

	struct {
		util::Cooldown run;
		util::Cooldown post_hurt;
		util::Cooldown post_shuriken_toss;
		util::Cooldown post_levitate;
		util::Cooldown start_levitate;
		util::Cooldown throw_shuriken;
	} m_cooldowns{};

	struct {
		util::Counter slam{};
	} m_counters{};

	struct {
		std::array<entity::Attack, 3> slash{};
		entity::Shockwave left_shockwave;
		entity::Shockwave right_shockwave;
	} m_attacks{};

	util::BitFlags<LynxFlags> m_flags{};
	void request(LynxState to) { m_state.desired = to; }
	bool change_state(LynxState next, anim::Parameters params);
	gui::BossHealth m_health_bar;
	vfx::Sparkler m_magic{};
	shape::Shape m_distant_range{};

	components::SteeringBehavior m_steering{};
	entity::Caution m_caution{};

	entity::WeaponPackage m_shuriken;
	sf::Vector2f m_attack_target{};

	std::optional<std::unique_ptr<gui::Console>>* m_console;
	automa::ServiceProvider* m_services;
	world::Map* m_map;

	sf::Vector2f m_player_target{};
	sf::Vector2f m_home{};
};

} // namespace fornani::enemy
