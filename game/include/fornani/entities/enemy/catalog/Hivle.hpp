#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/utils/Flaggable.hpp>

#define HIVLE_BIND(f) std::bind(&Hivle::f, this)

namespace fornani::enemy {

enum class HivleState : std::uint8_t { idle, turn, toss, run, jump, land };
enum class HivleVariant : std::uint8_t { javelin_tosser };
enum class HivleFlags : std::uint8_t { toss };

class Hivle final : public Enemy, public StateMachine<HivleState>, public Flaggable<HivleFlags> {
  public:
	Hivle(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	fsm::StateFunction state_function = std::bind(&Hivle::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_toss();
	fsm::StateFunction update_land();

  private:
	HivleVariant m_variant{};

	util::Cooldown m_toss_time;
	util::Cooldown m_switch_sides;
	std::optional<entity::WeaponPackage> m_javelin{};
	entity::Caution m_caution{};

	bool change_state(HivleState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
