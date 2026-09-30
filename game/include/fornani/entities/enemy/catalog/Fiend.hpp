#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/utils/Flaggable.hpp>

#define FIEND_BIND(f) std::bind(&Fiend::f, this)

namespace fornani::enemy {

enum class FiendState : std::uint8_t { idle, prepare_sweep, sweep, sidestep, prepare_uppercut, uppercut, turn };
enum class FiendVariant : std::uint8_t { spearman };
enum class FiendFlags : std::uint8_t { step_detected, projectile };

class Fiend final : public Enemy, public StateMachine<FiendState>, public Flaggable<FiendFlags> {
  public:
	Fiend(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	fsm::StateFunction state_function = std::bind(&Fiend::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_prepare_sweep();
	fsm::StateFunction update_sweep();
	fsm::StateFunction update_sidestep();
	fsm::StateFunction update_prepare_uppercut();
	fsm::StateFunction update_uppercut();
	fsm::StateFunction update_turn();

  private:
	std::optional<sf::Vector2f> find_teleport_position();
	void teleport();
	bool change_state(FiendState next, anim::Parameters params);

  private:
	FiendVariant m_variant{};

	sf::Vector2f m_player_position{};
	std::optional<sf::Vector2f> m_teleport_position{};

	util::Cooldown m_jump_time;
	util::Cooldown m_post_attack;
	util::Cooldown m_post_sidestep;
	util::Cooldown m_teleport;
	entity::Attack m_attack{};
	entity::Attack m_second_attack{};
	entity::Caution m_caution{};

	world::Map* m_map;
	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
