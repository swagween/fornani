#pragma once

#include "../Enemy.hpp"
#define CASTER_BIND(f) std::bind(&Caster::f, this)

namespace enemy {

enum class CasterState { idle, turn, signal, dormant };
enum class CasterVariant { apprentice, tyrant };

class Caster : public Enemy {

  public:
	Caster() = delete;
	~Caster() override {}
	Caster& operator=(Caster&&) = delete;
	Caster(automa::ServiceProvider& svc, world::Map& map);
	void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;
	[[nodiscard]] auto is_dormant() const -> bool { return state == CasterState::dormant || cooldowns.awaken.running(); }

	fsm::StateFunction state_function = std::bind(&Caster::update_dormant, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_signal();
	fsm::StateFunction update_dormant();

  private:
	CasterState state{};
	CasterVariant variant{};

	vfx::Gravitator target{};

	// packages
	struct {
		entity::FloatingPart scepter{};
		entity::FloatingPart wand{};
	} parts{};

	util::Cooldown hurt_effect{};

	struct {
		util::Cooldown post_cast{512};
		util::Cooldown awaken{180};
	} cooldowns{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 4, 28, -1};
	anim::Parameters turn{9, 3, 18, 0};
	anim::Parameters signal{4, 4, 28, 2};
	anim::Parameters dormant{8, 1, 32, -1};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(CasterState next, anim::Parameters params);
};

} // namespace enemy