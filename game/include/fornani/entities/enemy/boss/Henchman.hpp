#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/entities/packages/Shockwave.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/particle/Sparkler.hpp>

#define HENCHMAN_BIND(f) std::bind(&Henchman::f, this)

namespace fornani::enemy {

enum class HenchmanState : std::uint8_t { idle, jump, hop, land, forward_slash, jumpsquat, whistle, knife_toss, prepare_downward_cut, prepare_upward_cut, downward_cut, upward_cut, turn, back_hop };
enum class HenchmanFlags : std::uint8_t { whiffed, retreated };

class Henchman final : public Boss, public StateMachine<HenchmanState> {
  public:
	Henchman(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Henchman::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_land();
	fsm::StateFunction update_forward_slash();
	fsm::StateFunction update_knife_toss();
	fsm::StateFunction update_prepare_downward_cut();
	fsm::StateFunction update_prepare_upward_cut();
	fsm::StateFunction update_downward_cut();
	fsm::StateFunction update_upward_cut();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_hop();
	fsm::StateFunction update_back_hop();
	fsm::StateFunction update_jumpsquat();
	fsm::StateFunction update_whistle();

  private:
	void debug();
	[[nodiscard]] auto is_airborne() const -> bool { return is_state(HenchmanState::jump) || is_state(HenchmanState::hop); }

  private:
	struct {
		util::Cooldown post_slash{600};
		util::Cooldown post_cut{600};
		util::Cooldown post_whistle{2600};
	} m_cooldowns{};

	struct {
		std::array<entity::Attack, 3> slash{};
		entity::Shockwave left_shockwave;
		entity::Shockwave right_shockwave;
	} m_attacks{};

	util::BitFlags<HenchmanFlags> m_flags{};

	bool change_state(HenchmanState next, anim::Parameters params);

	entity::Caution m_caution{};

	entity::WeaponPackage m_slash_wave;
	components::SteeringBehavior m_steering{};
	sf::Vector2f m_steer_target{};
	sf::Vector2f m_boundary{};

	automa::ServiceProvider* m_services;
	world::Map* m_map;
};

} // namespace fornani::enemy
