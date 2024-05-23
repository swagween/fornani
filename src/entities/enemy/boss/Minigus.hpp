#pragma once

#include "../Enemy.hpp"
#include "../../npc/NPC.hpp"
#define MINIGUS_BIND(f) std::bind(&Minigus::f, this)

namespace enemy {

	enum class MinigusState { idle, turn, run, shoot, jump_shoot, hurt, jump, jumpsquat, reload };
	enum class MinigunState {deactivated, neutral, charging, firing};

class Minigus : public Enemy, public npc::NPC {

	  public:
		Minigus() = default;
		Minigus(automa::ServiceProvider& svc);
		void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
		void unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;

		fsm::StateFunction state_function = std::bind(&Minigus::update_idle, this);
		fsm::StateFunction update_idle();
		fsm::StateFunction update_shoot();
		fsm::StateFunction update_jumpsquat();
		fsm::StateFunction update_hurt();
		fsm::StateFunction update_jump();
		fsm::StateFunction update_jump_shoot();
		fsm::StateFunction update_reload();
		fsm::StateFunction update_turn();
		fsm::StateFunction update_run();

	  private:
		MinigusState state{};

		struct {
			sf::Sprite sprite{};
			anim::Animation animation{};
			MinigunState state{};
			anim::Parameters neutral{0, 4, 36, -1};
			anim::Parameters deactivated{11, 2, 38, -1};
			anim::Parameters charging{21, 11, 28, -1};
			anim::Parameters firing{13, 8, 20, -1};
			sf::Vector2<float> offset{-26, 40};
		} minigun{};


	int fire_chance{80};

	//packages
	entity::WeaponPackage gun;
	entity::Caution caution{};
	util::Cooldown running_time{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 2, 48, -1};
	anim::Parameters shoot{2, 2, 38, 8};
	anim::Parameters jumpsquat{4, 1, 38, 1};
	anim::Parameters hurt{5, 4, 38, 4};
	anim::Parameters jump{9, 1, 22, -1};
	anim::Parameters jump_shoot{10, 2, 42, -1};
	anim::Parameters reload{12, 7, 42, 1};
	anim::Parameters turn{19, 2, 42, 1};
	anim::Parameters run{21, 4, 42, -1};

	automa::ServiceProvider* m_services;

	bool change_state(MinigusState next, anim::Parameters params);

};

} // namespace enemy