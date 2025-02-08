#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/Attack.hpp"
#include "fornani/entities/packages/Caution.hpp"
#define THUG_BIND(f) std::bind(&Thug::f, this)

namespace fornani::enemy {

enum class ThugState { idle, turn, run, jump, alert, rush, punch };

class Thug : public Enemy {

	  public:
		Thug() = delete;
		~Thug() override {}
		Thug& operator = (Thug&&) = delete;
		Thug(automa::ServiceProvider& svc, world::Map& map);
		void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
		void unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;

		fsm::StateFunction state_function = std::bind(&Thug::update_idle, this);
		fsm::StateFunction update_idle();
		fsm::StateFunction update_turn();
		fsm::StateFunction update_run();
		fsm::StateFunction update_jump();
		fsm::StateFunction update_alert();
		fsm::StateFunction update_rush();
		fsm::StateFunction update_punch();

	  private:
		ThugState state{};


	//packages
		struct {
			entity::Attack punch{};
			entity::Attack rush{};
		} attacks{};
	entity::Caution caution{};

	struct {
		util::Cooldown jump{40};
		util::Cooldown rush_hit{600};
	} cooldowns{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 4, 28, -1};
	anim::Parameters turn{22, 2, 38, 0};
	anim::Parameters run{4, 4, 28, 4};
	anim::Parameters jump{6, 1, 22, -1};
	anim::Parameters alert{8, 4, 42, 0};
	anim::Parameters rush{12, 4, 22, 3};
	anim::Parameters punch{16, 6, 32, 0};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(ThugState next, anim::Parameters params);

};

} // namespace enemy