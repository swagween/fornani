
#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/entities/packages/Shockwave.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/particle/Sparkler.hpp>

#define HAUNCH_BIND(f) std::bind(&Haunch::f, this)

namespace fornani::enemy {

enum class HaunchState { idle, airborne, turn, shoot_high, shoot_low, get_up, walk, pull_grenade, throw_grenade, throw_grenade_down, triple_toss, whistle, triple_down_toss, struggle, stalk, rage_turn, rage_struggle };
enum class HaunchFlags { laser_fired, jumped, show_gun, escape_cutscene_launched, started_struggle };

class Haunch final : public Boss, public StateMachine<HaunchState> {
  public:
	Haunch(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	fsm::StateFunction state_function = std::bind(&Haunch::update_airborne, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_airborne();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_shoot_high();
	fsm::StateFunction update_shoot_low();
	fsm::StateFunction update_get_up();
	fsm::StateFunction update_walk();
	fsm::StateFunction update_pull_grenade();
	fsm::StateFunction update_throw_grenade();
	fsm::StateFunction update_triple_toss();
	fsm::StateFunction update_throw_grenade_down();
	fsm::StateFunction update_whistle();
	fsm::StateFunction update_triple_down_toss();
	fsm::StateFunction update_struggle();
	fsm::StateFunction update_stalk();
	fsm::StateFunction update_rage_turn();
	fsm::StateFunction update_rage_struggle();

  private:
	void shoot_gun();

  private:
	struct {
		util::Cooldown run;
		util::Cooldown post_run;
		util::Cooldown grenade;
		util::Cooldown laser_charge;
		util::Cooldown post_laser;
		util::Cooldown whistle;
		util::Cooldown post_whistle;
		util::Cooldown post_death;
	} m_cooldowns{};

	util::BitFlags<HaunchFlags> m_flags{};

	Animatable m_laser_gun;
	std::optional<Animatable> m_dynamite_stick;

	bool change_state(HaunchState next, anim::Parameters params);

	components::SteeringComponent m_gun_steering{};
	entity::Caution m_caution{};

	entity::WeaponPackage m_stun_grenade;
	entity::WeaponPackage m_hand_grenade;
	entity::WeaponPackage m_gun;
	sf::Vector2f m_attack_target{};
	sf::Vector2f m_gun_socket;

	automa::ServiceProvider* m_services;
	world::Map* m_map;
};

} // namespace fornani::enemy
