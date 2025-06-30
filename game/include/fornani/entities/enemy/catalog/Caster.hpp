#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/FloatingPart.hpp"
#define CASTER_BIND(f) std::bind(&Caster::f, this)

namespace fornani::enemy {

enum class CasterState { idle, turn, signal, dormant, prepare };
enum class CasterVariant { apprentice, tyrant };

class Caster final : public Enemy {

  public:
	Caster() = delete;
	~Caster() override {}
	Caster& operator=(Caster&&) = delete;
	Caster(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;
	void teleport();
	[[nodiscard]] auto is_dormant() const -> bool { return state == CasterState::dormant || cooldowns.awaken.running(); }

	fsm::StateFunction state_function = std::bind(&Caster::update_dormant, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_prepare();
	fsm::StateFunction update_signal();
	fsm::StateFunction update_dormant();

  private:
	CasterState state{};
	CasterVariant variant{};

	vfx::Gravitator target{};
	entity::WeaponPackage energy_ball;
	sf::Vector2<float> attack_target{};

	// packages
	struct {
		entity::FloatingPart scepter;
		entity::FloatingPart wand;
	} parts;

	struct {
		util::Cooldown post_cast{512};
		util::Cooldown awaken{180};
		util::Cooldown pre_invisibility{32};
		util::Cooldown invisibility{96};
		util::Cooldown rapid_fire{58};
	} cooldowns{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 4, 28, -1};
	anim::Parameters turn{9, 3, 18, 0};
	anim::Parameters prepare{9, 3, 18, 0};
	anim::Parameters signal{4, 4, 28, 2};
	anim::Parameters dormant{8, 1, 32, -1};

	sf::Vector2<int> wand_dimensions{46, 62};
	sf::Vector2<int> scepter_dimensions{122, 20};
	util::Cycle flash{2};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(CasterState next, anim::Parameters params);
};

} // namespace enemy