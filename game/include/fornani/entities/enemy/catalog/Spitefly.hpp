
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Caution.hpp>

#define SPITEFLY_BIND(f) std::bind(&Spitefly::f, this)

namespace fornani::enemy {

enum class SpiteflyState : std::uint8_t { idle, turn, sleep, awaken };
enum class SpiteflyVariant : std::uint8_t { bat, bomb, albino };

class Spitefly final : public Enemy, public StateMachine<SpiteflyState> {
  public:
	Spitefly(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	[[nodiscard]] auto is_bomb() const -> bool { return m_variant == SpiteflyVariant::bomb; }
	[[nodiscard]] auto is_albino() const -> bool { return m_variant == SpiteflyVariant::albino; }
	[[nodiscard]] auto is_active() const -> bool { return !is_state(SpiteflyState::sleep) && !is_state(SpiteflyState::awaken) && is_alert(); }

	fsm::StateFunction state_function = std::bind(&Spitefly::update_sleep, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_sleep();
	fsm::StateFunction update_awaken();

  private:
	SpiteflyVariant m_variant{};

	components::SteeringBehavior m_steering{};

	std::optional<entity::FloatingPart> m_bomb_part{};
	std::optional<entity::WeaponPackage> m_bomb{};

	bool change_state(SpiteflyState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
