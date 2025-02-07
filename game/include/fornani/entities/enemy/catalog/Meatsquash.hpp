#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#define MEATSQUASH_BIND(f) std::bind(&Meatsquash::f, this)

namespace fornani::enemy {

enum class MeatsquashState { idle, chomp, swallow, open };

class Meatsquash : public Enemy {

  public:
	Meatsquash() = delete;
	~Meatsquash() override {}
	Meatsquash& operator=(Meatsquash&&) = delete;
	Meatsquash(automa::ServiceProvider& svc, world::Map& map);
	void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;

	fsm::StateFunction state_function = std::bind(&Meatsquash::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_chomp();
	fsm::StateFunction update_swallow();
	fsm::StateFunction update_open();

  private:
	MeatsquashState state{};

	//attack
	struct {
		entity::Attack bite{};
	} attacks{};
	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 6, 36, -1};
	anim::Parameters chomp{6, 12, 36, 0};
	anim::Parameters open{19, 4, 36, 0};
	anim::Parameters swallow{23, 13, 36, 0};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(MeatsquashState next, anim::Parameters params);
};

} // namespace enemy