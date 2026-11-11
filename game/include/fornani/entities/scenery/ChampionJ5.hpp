
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/entities/Mobile.hpp>
#include <fornani/entities/animation/StateMachine.hpp>
#include <fornani/entities/packages/Health.hpp>
#include <fornani/gui/BossHealth.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/shader/FlatShader.hpp>
#include <fornani/utils/StateFunction.hpp>
#define CHAMPIONJ5_BIND(f) std::bind(&ChampionJ5::f, this)

namespace fornani::arms {
class Projectile;
}

namespace fornani {

enum class ChampionJ5State : std::uint8_t { flying, land, grounded, take_off };
enum class ChampionJ5Flags : std::uint8_t { interactable, landed, controlled, battle };

class ChampionJ5 : public Mobile, public StateMachine<ChampionJ5State> {
  public:
	ChampionJ5(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void gui_render(sf::RenderWindow& win);
	void set_target(sf::Vector2f to) { m_target = to; }
	void set_position(sf::Vector2f to) { get_collider().physics.position = to; }
	void set_channel(int to) { p_animatable.set_channel(to); }
	void on_hit(arms::Projectile& proj, world::Map& map);
	void set_battle(bool const to = true);

	fsm::StateFunction state_function = std::bind(&ChampionJ5::update_flying, this);
	fsm::StateFunction update_flying();
	fsm::StateFunction update_land();
	fsm::StateFunction update_grounded();
	fsm::StateFunction update_take_off();

	util::BitFlags<ChampionJ5Flags> flags{};

	[[nodiscard]] auto get_drivers_seat() const -> sf::Vector2f;
	[[nodiscard]] auto get_passengers_seat() const -> sf::Vector2f;
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return get_collider().get_position(); }
	[[nodiscard]] auto is_close_to_target(float const distance) const -> bool { return (get_collider().physics.position - m_target).length() < distance; }
	[[nodiscard]] auto is_dead() const -> bool { return m_health.is_dead(); }

  private:
	bool change_state(ChampionJ5State next, anim::Parameters params);
	float get_y_offset() const;

	void debug();

  private:
	Health m_health;
	gui::BossHealth m_health_bar;
	Animatable m_propeller;
	components::SteeringBehavior m_steering{};
	sf::Vector2f m_target{};
	ThrustParameters m_thrust;
	shape::Shape m_hurtbox;
	util::Cooldown m_hit;

	FlatShader m_flat_shader;

	automa::ServiceProvider* m_services;

	io::Logger m_logger{"Scenery"};
};

} // namespace fornani
