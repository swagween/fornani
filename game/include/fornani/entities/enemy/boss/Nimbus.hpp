
#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/entities/packages/Shockwave.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/particle/Sparkler.hpp>

#define NIMBUS_BIND(f) std::bind(&Nimbus::f, this)

namespace fornani::enemy {

enum class NimbusState : std::uint8_t { idle, jump, land, get_up, prepare_forward_slash, forward_slash, prepare_twirl_cut, twirl_cut, flourish, downward_cut, upward_cut, turn, parry, begin_levitate, levitate, spellcast, double_aerial_slash };
enum class NimbusFlags : std::uint8_t {};

class Nimbus final : public Boss, public StateMachine<NimbusState> {
  public:
	Nimbus(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Nimbus::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_land();
	fsm::StateFunction update_get_up();
	fsm::StateFunction update_prepare_forward_slash();
	fsm::StateFunction update_forward_slash();
	fsm::StateFunction update_prepare_twirl_cut();
	fsm::StateFunction update_twirl_cut();
	fsm::StateFunction update_flourish();
	fsm::StateFunction update_downward_cut();
	fsm::StateFunction update_upward_cut();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_parry();
	fsm::StateFunction update_begin_levitate();
	fsm::StateFunction update_levitate();
	fsm::StateFunction update_spellcast();
	fsm::StateFunction update_double_aerial_slash();

  private:
	void debug();
	[[nodiscard]] auto is_airborne() const -> bool { return is_state(NimbusState::begin_levitate) || is_state(NimbusState::levitate); }

  private:
	struct {
		util::Cooldown post_slash{600};
	} m_cooldowns{};

	struct {
		std::array<entity::Attack, 3> slash{};
	} m_attacks{};

	util::BitFlags<NimbusFlags> m_flags{};

	bool change_state(NimbusState next, anim::Parameters params);

	entity::Caution m_caution{};

	entity::WeaponPackage m_slash_wave;
	components::SteeringBehavior m_steering{};
	sf::Vector2f m_steer_target{};

	automa::ServiceProvider* m_services;
	world::Map* m_map;
};

} // namespace fornani::enemy
