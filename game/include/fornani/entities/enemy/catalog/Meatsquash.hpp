
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/utils/Flaggable.hpp>
#define MEATSQUASH_BIND(f) std::bind(&Meatsquash::f, this)

namespace fornani::enemy {

enum class MeatsquashState { idle, chomp, swallow, open };
enum class MeatsquashFlags { swallowed_player, chomped };

class Meatsquash final : public Enemy, public StateMachine<MeatsquashState>, public Flaggable<MeatsquashFlags> {

  public:
	Meatsquash(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Meatsquash::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_chomp();
	fsm::StateFunction update_swallow();
	fsm::StateFunction update_open();

  private:
	// attack
	struct {
		entity::Attack bite{};
	} attacks{};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(MeatsquashState next, anim::Parameters params);
};

} // namespace fornani::enemy
