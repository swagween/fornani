
#pragma once

#include "fornani/entities/animation/Animation.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/utils/Counter.hpp"
#include "fornani/utils/StateFunction.hpp"
#define PA_BIND(f) std::bind(&PlayerAnimation::f, this)

namespace fornani::player {

class Player;

enum class AnimState : std::uint8_t {
	idle,
	turn,
	sharp_turn,
	run,
	sprint,
	shield,
	between_push,
	push,
	rise,
	suspend,
	fall,
	stop,
	inspect,
	sit,
	land,
	hurt,
	dash,
	wallslide,
	walljump,
	die,
	backflip,
	slide,
	get_up,
	roll,
	shoot,
	sleep,
	wake_up
};
enum class AnimTriggers : std::uint8_t { flip, end_death };

class PlayerAnimation {
  private:
	std::unordered_map<std::string, anim::Parameters> m_params;

  public:
	friend class Player;
	PlayerAnimation(player::Player& plr);

	anim::Animation animation{};
	AnimState state{};
	util::BitFlags<AnimTriggers> triggers{};
	util::Counter idle_timer{};
	util::Cooldown post_death{400};
	struct {
		util::Cooldown walljump{24};
	} cooldowns{};

	void update();
	void start();
	[[nodiscard]] auto death_over() -> bool { return triggers.consume(AnimTriggers::end_death); }
	[[nodiscard]] auto not_jumping() const -> bool { return state != AnimState::rise; }
	[[nodiscard]] auto get_frame() const -> int { return animation.get_frame(); }
	[[nodiscard]] auto was_requested(AnimState check) const -> bool { return m_requested.test(check); }
	[[nodiscard]] auto get_state() const -> AnimState { return m_actual; }
	bool stepped() const;

	fsm::StateFunction state_function;

	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_sharp_turn();
	fsm::StateFunction update_sprint();
	fsm::StateFunction update_shield();
	fsm::StateFunction update_between_push();
	fsm::StateFunction update_push();
	fsm::StateFunction update_run();
	fsm::StateFunction update_rise();
	fsm::StateFunction update_suspend();
	fsm::StateFunction update_fall();
	fsm::StateFunction update_stop();
	fsm::StateFunction update_inspect();
	fsm::StateFunction update_sit();
	fsm::StateFunction update_land();
	fsm::StateFunction update_hurt();
	fsm::StateFunction update_dash();
	fsm::StateFunction update_wallslide();
	fsm::StateFunction update_walljump();
	fsm::StateFunction update_die();
	fsm::StateFunction update_backflip();
	fsm::StateFunction update_slide();
	fsm::StateFunction update_get_up();
	fsm::StateFunction update_roll();
	fsm::StateFunction update_shoot();
	fsm::StateFunction update_sleep();
	fsm::StateFunction update_wake_up();

	bool change_state(AnimState next, anim::Parameters params, bool hard = false);
	void request_animation(AnimState to_state) { m_requested.set(to_state); };

	Player* m_player;

	struct {
		int sit{2400};
	} timers{};

  private:
	anim::Parameters const& get_params(std::string const& key);
	util::BitFlags<AnimState> m_requested{};
	AnimState m_actual{};

	io::Logger m_logger{"animation"};
};

} // namespace fornani::player
