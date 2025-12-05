
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Caution.hpp>

#define MINION_BIND(f) std::bind(&Minion::f, this)

namespace fornani::enemy {

enum class MinionState { idle, blink, jump, turn };

class Minion final : public Enemy {
  public:
	Minion(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	[[nodiscard]] auto is_state(MinionState test) const -> bool { return m_state.actual == test; }

	fsm::StateFunction state_function = std::bind(&Minion::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_blink();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_turn();

  private:
	struct {
		MinionState actual{};
		MinionState desired{};
	} m_state{};

	util::Cooldown m_jump;
	util::Cooldown m_tick;

	entity::Caution m_caution{};

	void request(MinionState to) { m_state.desired = to; }
	bool change_state(MinionState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
