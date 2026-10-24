
#pragma once

#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/entities/packages/Shockwave.hpp>
#include <fornani/entities/packages/WeaponPackage.hpp>
#include <fornani/utils/Flaggable.hpp>

#define SLUDGELORD_BIND(f) std::bind(&Sludgelord::f, this)

namespace fornani::enemy {

enum class SludgelordState : std::uint8_t { idle, turn, spit, roll, slam, bounce, roar, spew };
enum class SludgelordFlags : std::uint8_t { spit, slamming, landed, bounced };

class Sludgelord final : public Boss, public StateMachine<SludgelordState> {
  public:
	Sludgelord(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	fsm::StateFunction state_function = std::bind(&Sludgelord::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_spit();
	fsm::StateFunction update_roll();
	fsm::StateFunction update_slam();
	fsm::StateFunction update_bounce();
	fsm::StateFunction update_roar();
	fsm::StateFunction update_spew();

  private:
	util::BitFlags<SludgelordFlags> m_flags{};
	components::SteeringBehavior m_steering{};
	util::Cooldown m_spit_time;
	util::Cooldown m_switch_sides;
	util::Cooldown m_slam;
	util::Counter m_bounce_counter{};
	std::optional<entity::WeaponPackage> m_bomb{};
	entity::WeaponPackage m_spew;
	std::vector<entity::Shockwave> m_shockwaves{};

	shape::CircleCollider m_bounce_collider{50.f};

	bool change_state(SludgelordState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
