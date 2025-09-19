
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/npc/NPC.hpp>
#include <fornani/gui/BossHealth.hpp>
#include <fornani/particle/Sparkler.hpp>

#define LYNX_BIND(f) std::bind(&Lynx::f, this)

namespace fornani::enemy {

enum class LynxState : std::uint8_t { sit, get_up, idle, jump, forward_slash, levitate, run, downward_slam, prepare_shuriken, toss_shuriken, upward_slash, triple_slash, turn };
enum class LynxFlags : std::uint8_t { conversing, battle_mode, second_phase };

class Lynx final : public Enemy, public npc::NPC {
  public:
	Lynx(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	[[nodiscard]] auto invincible() const -> bool { return !flags.state.test(StateFlags::vulnerable); }
	[[nodiscard]] auto half_health() const -> bool { return health.get_hp() < health.get_max() * 0.5f; }

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

  private:
	struct {
		LynxState actual{};
		LynxState desired{};
	} m_state{};
	struct {
		util::Cooldown run;
	} m_cooldowns{};
	util::BitFlags<LynxFlags> m_flags{};
	void request(LynxState to) { m_state.desired = to; }
	bool change_state(LynxState next, anim::Parameters params);
	gui::BossHealth m_health_bar;
	vfx::Sparkler m_magic{};

	components::SteeringBehavior m_steering{};

	std::optional<std::unique_ptr<gui::Console>>* m_console;
	automa::ServiceProvider* m_services;

	std::unordered_map<std::string, anim::Parameters> m_params;
	anim::Parameters const& get_params(std::string const& key);
};

} // namespace fornani::enemy
