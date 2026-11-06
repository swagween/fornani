
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/WeaponPackage.hpp>

#define DOGFIGHT_HULMET_BIND(f) std::bind(&DogfightHulmet::f, this)

namespace fornani::enemy {

enum class DogfightHulmetState : std::uint8_t { idle };

class DogfightHulmet final : public Enemy, public StateMachine<DogfightHulmetState> {
  public:
	DogfightHulmet(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&DogfightHulmet::update_idle, this);
	fsm::StateFunction update_idle();

  private:
	bool change_state(DogfightHulmetState next, anim::Parameters params);

	void debug();

  private:
	entity::WeaponPackage m_turret;
	components::SteeringBehavior m_steering{};

	bool m_init{};

	float m_rotation{};
	PursuitParameters m_pursuit_parameters;
	sf::Vector2f m_offset{};

	world::Map* m_map;
	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
