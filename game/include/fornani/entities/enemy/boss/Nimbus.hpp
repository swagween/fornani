
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

enum class NimbusState : std::uint8_t {
	idle,
	jump,
	land,
	get_up,
	prepare_forward_slash,
	forward_slash,
	prepare_twirl_cut,
	twirl_cut,
	flourish,
	downward_cut,
	upward_cut,
	turn,
	parry,
	begin_levitate,
	levitate,
	spellcast,
	double_aerial_slash,
	aerial_slash,
	frenzy_prepare,
	frenzy_quick_prepare,
	frenzy_dodge_prepare,
	frenzy_dash,
	frenzy_cuts,
	stagger,
	teleport,
	begin_laugh,
	laugh
};
enum class NimbusFlags : std::uint8_t { frenzy_contact, short_jump, successful_hit, victory, staggered, just_teleported, killed_player };

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
	fsm::StateFunction update_aerial_slash();
	fsm::StateFunction update_frenzy_prepare();
	fsm::StateFunction update_frenzy_quick_prepare();
	fsm::StateFunction update_frenzy_dodge_prepare();
	fsm::StateFunction update_frenzy_dash();
	fsm::StateFunction update_frenzy_cuts();
	fsm::StateFunction update_stagger();
	fsm::StateFunction update_teleport();
	fsm::StateFunction update_begin_laugh();
	fsm::StateFunction update_laugh();

  private:
	void debug();
	[[nodiscard]] auto is_airborne() const -> bool { return is_state(NimbusState::begin_levitate) || is_state(NimbusState::levitate); }

  private:
	struct {
		util::Cooldown post_slash{600};
		util::Cooldown parry{800};
		util::Cooldown post_jump{1800};
		util::Cooldown teleport{60};
		util::Cooldown spellcast{200};
	} m_cooldowns{};

	struct {
		std::array<entity::Attack, 3> slash{};
		entity::Attack frenzy{};
		shape::CircleCollider frenzy_collider{40.f};
	} m_attacks{};

	struct {
		shape::Shape bounding_box{{48.f, 512.f}};
		util::Counter counter{};
		util::Cooldown active{190};
		sf::Vector2f target{};
		components::SteeringComponent steering{};
	} m_lightning{};

	util::Counter m_triple_jump{};
	util::Counter m_quick_teleports{};

	util::BitFlags<NimbusFlags> m_flags{};

	bool change_state(NimbusState next, anim::Parameters params);

	entity::Caution m_caution{};

	entity::WeaponPackage m_slash_wave;
	components::SteeringBehavior m_steering{};
	sf::Vector2f m_steer_target{};
	sf::Vector2f m_player_position{};
	sf::Vector2f m_previous_home_point{};

	automa::ServiceProvider* m_services;
	world::Map* m_map;
};

} // namespace fornani::enemy
