
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/particle/Chain.hpp>
#define MACROPHAGE_BIND(f) std::bind(&Macrophage::f, this)

namespace fornani::enemy {

enum class MacrophageState { idle };
enum class MacrophageFlags { caught_player, released_player };

class Macrophage final : public Enemy, public StateMachine<MacrophageState> {

  public:
	Macrophage(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Macrophage::update_idle, this);
	fsm::StateFunction update_idle();

  private:
	automa::ServiceProvider* m_services;
	world::Map* m_map;

	components::SteeringBehavior m_steering{};
	util::BitFlags<MacrophageFlags> m_flags{};
	entity::Attack m_grab{};
	std::unique_ptr<vfx::Chain> m_body;

	bool change_state(MacrophageState next, anim::Parameters params);
};

} // namespace fornani::enemy
