#pragma once

#include "../Enemy.hpp"
#define ARCHER_BIND(f) std::bind(&Archer::f, this)

namespace enemy {

enum class ArcherState { idle, turn, run, jump, shoot };
enum class ArcherVariant { huntress, defender };

class Archer : public Enemy {

  public:
	Archer() = delete;
	~Archer() override {}
	Archer& operator=(Archer&&) = delete;
	Archer(automa::ServiceProvider& svc, world::Map& map);
	void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;

	fsm::StateFunction state_function = std::bind(&Archer::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_shoot();

  private:
	ArcherState state{};
	ArcherVariant variant{};

	// packages
	struct {
		entity::FloatingPart bow;
	} parts;
	sf::Vector2<int> bow_dimensions{82, 82};

	entity::Caution caution{};

	struct {
		util::Cooldown jump{40};
		util::Cooldown post_jump{400};
	} cooldowns{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 8, 28, -1};
	anim::Parameters turn{8, 1, 28, 0};
	anim::Parameters run{9, 4, 28, 4};
	anim::Parameters jump{9, 1, 48, 0};
	anim::Parameters shoot{13, 1, 164, 0};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	float rand_jump{};

	bool change_state(ArcherState next, anim::Parameters params);
};

} // namespace enemy