#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/Caution.hpp"
#include "fornani/entities/packages/FloatingPart.hpp"
#define ARCHER_BIND(f) std::bind(&Archer::f, this)

namespace fornani::enemy {

enum class ArcherState { idle, turn, run, jump, shoot };
enum class ArcherVariant { huntress, defender };

class Archer final : public Enemy {

  public:
	Archer() = delete;
	~Archer() override {}
	Archer& operator=(Archer&&) = delete;
	Archer(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	[[nodiscard]] auto is_state(ArcherState test) const -> bool { return m_state.actual == test; }

	fsm::StateFunction state_function = std::bind(&Archer::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_shoot();

  private:
	ArcherVariant variant{};
	entity::WeaponPackage m_bow;

	struct {
		ArcherState actual{};
		ArcherState desired{};
	} m_state{};

	// packages
	struct {
		entity::FloatingPart bow;
	} parts;
	sf::Vector2<int> bow_dimensions{41, 41};

	entity::Caution caution{};

	struct {
		util::Cooldown jump{40};
		util::Cooldown post_jump{400};
	} cooldowns{};

	sf::Vector2f m_player_target{};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	float rand_jump{};

	void request(ArcherState to) { m_state.desired = to; }
	bool change_state(ArcherState next, anim::Parameters params);
};

} // namespace fornani::enemy
