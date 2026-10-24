#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/entities/packages/Shockwave.hpp>
#include <fornani/entities/packages/WeaponPackage.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/particle/Sparkler.hpp>

#define HOARDER_BIND(f) std::bind(&Hoarder::f, this)

namespace fornani::enemy {

enum class HoarderState : std::uint8_t { idle, burrow, rummage, emerge, bg_burrow, bg_rummage, bg_emerge, bg_alert, present, hop_on_bike, ride_bike, tantrum, die, run, laugh, cry, get_up };
enum class HoarderFlags : std::uint8_t { toss, retreated };

class Hoarder final : public Boss, public StateMachine<HoarderState> {
  public:
	Hoarder(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Hoarder::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_burrow();
	fsm::StateFunction update_rummage();
	fsm::StateFunction update_emerge();
	fsm::StateFunction update_bg_burrow();
	fsm::StateFunction update_bg_rummage();
	fsm::StateFunction update_bg_emerge();
	fsm::StateFunction update_bg_alert();
	fsm::StateFunction update_present();
	fsm::StateFunction update_hop_on_bike();
	fsm::StateFunction update_ride_bike();
	fsm::StateFunction update_tantrum();
	fsm::StateFunction update_die();
	fsm::StateFunction update_run();
	fsm::StateFunction update_laugh();
	fsm::StateFunction update_cry();
	fsm::StateFunction update_get_up();

  private:
	void debug();

  private:
	struct {
	} m_cooldowns{};

	struct {
		std::array<entity::Attack, 3> slash{};
		std::vector<entity::Shockwave> shockwaves{};
	} m_attacks{};

	util::BitFlags<HoarderFlags> m_flags{};

	bool change_state(HoarderState next, anim::Parameters params);

	entity::Caution m_caution{};

	entity::WeaponPackage m_slash_wave;
	entity::WeaponPackage m_trash_toss;
	components::SteeringBehavior m_steering{};
	sf::Vector2f m_steer_target{};
	sf::Vector2f m_boundary{};
	sf::Vector2f m_previous_home_point{};

	automa::ServiceProvider* m_services;
	world::Map* m_map;
};

} // namespace fornani::enemy
