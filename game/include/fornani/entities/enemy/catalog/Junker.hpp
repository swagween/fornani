
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/utils/Flaggable.hpp>

#define JUNKER_BIND(f) std::bind(&Junker::f, this)

namespace fornani::enemy {

enum class JunkerState { idle, turn, toss, hide };
enum class JunkerVariant { grenade };
enum class JunkerFlags { toss };

class Junker final : public Enemy, public StateMachine<JunkerState>, public Flaggable<JunkerFlags> {
  public:
	Junker(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	fsm::StateFunction state_function = std::bind(&Junker::update_hide, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_toss();
	fsm::StateFunction update_hide();

  private:
	JunkerVariant m_variant{};

	components::SteeringBehavior m_steering{};
	entity::Caution m_caution{};
	util::Cooldown m_toss_time;
	std::optional<entity::WeaponPackage> m_bomb{};

	bool change_state(JunkerState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
