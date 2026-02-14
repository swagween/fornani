
#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entities/animation/StateMachine.hpp>
#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/FloatingPart.hpp>
#define CASTER_BIND(f) std::bind(&Caster::f, this)

namespace fornani::enemy {

enum class CasterState { idle, turn, signal, dormant, prepare };
enum class CasterVariant { apprentice, tyrant };

class Caster final : public Enemy, StateMachine<CasterState> {

  public:
	Caster(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void teleport();
	[[nodiscard]] auto is_dormant() const -> bool { return is_state(CasterState::dormant) || cooldowns.awaken.running(); }

	fsm::StateFunction state_function = std::bind(&Caster::update_dormant, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_prepare();
	fsm::StateFunction update_signal();
	fsm::StateFunction update_dormant();

  private:
	CasterVariant m_variant{};

	components::SteeringBehavior m_steering{};
	entity::WeaponPackage energy_ball;
	sf::Vector2f attack_target{};
	float m_target_force{};

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

	sf::Vector2<int> wand_dimensions{23, 31};
	sf::Vector2<int> scepter_dimensions{61, 10};
	util::Cycle flash{2};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(CasterState next, anim::Parameters params);

	void debug();
	bool m_debug;
};

} // namespace fornani::enemy
