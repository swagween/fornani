#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/entities/packages/Shockwave.hpp>
#include <fornani/entities/packages/WeaponPackage.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/particle/Sparkler.hpp>

#define DOGFIGHT_NIMBUS_BIND(f) std::bind(&DogfightNimbus::f, this)

namespace fornani::enemy {

enum class DogfightNimbusState : std::uint8_t { idle, turn_down, turn_up };
enum class DogfightNimbusFlags : std::uint8_t {};

class DogfightNimbus final : public Boss, public StateMachine<DogfightNimbusState> {
  public:
	DogfightNimbus(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&DogfightNimbus::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn_down();
	fsm::StateFunction update_turn_up();

  private:
	bool change_state(DogfightNimbusState next, anim::Parameters params);
	void debug();

  private:
	entity::WeaponPackage m_turret;
	entity::Caution m_caution;
	components::SteeringBehavior m_steering{};
	util::Cooldown m_recharge;
	util::Cooldown m_fire;
	util::Cooldown m_rounds;
	util::Cooldown m_barrel_roll;

	float m_turn_factor{0.2f};
	float m_rotation{};
	float m_flip_threshold{60.f};

	SimpleDirection m_current_side{LR::left};
	PursuitParameters m_pursuit_parameters;
	sf::Vector2f m_offset{};
};

} // namespace fornani::enemy
