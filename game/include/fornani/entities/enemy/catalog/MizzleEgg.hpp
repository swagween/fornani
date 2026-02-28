
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/particle/Spring.hpp>

#define MIZZLE_EGG_BIND(f) std::bind(&MizzleEgg::f, this)

namespace fornani::enemy {

enum class MizzleEggState { closed, open, in_between, hatch };
enum class MizzleEggFlags { detached, hatched };

class MizzleEgg final : public Enemy, public StateMachine<MizzleEggState> {
  public:
	MizzleEgg(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&MizzleEgg::update_closed, this);
	fsm::StateFunction update_closed();
	fsm::StateFunction update_in_between();
	fsm::StateFunction update_open();
	fsm::StateFunction update_hatch();

  private:
	void set_root(world::Map& map);
	void spawn_mizzle();

  private:
	world::Map* m_map;
	vfx::Spring* m_root{};
	util::Cooldown init{80};
	util::Cooldown m_hatch_timer;
	util::Cooldown m_mizzle_spawn;

	util::BitFlags<MizzleEggFlags> m_flags{};

	components::SteeringBehavior m_steering{};

	bool change_state(MizzleEggState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
