
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#define THIEF_BIND(f) std::bind(&Thief::f, this)

namespace fornani::enemy {

enum class ThiefState : std::uint8_t { dive, land, prepare, dash, stop, laugh, escape, hurt };
enum class ThiefFlags : std::uint8_t { succeeded, dive_in, hiding };

class Thief final : public Enemy, public StateMachine<ThiefState> {

  public:
	Thief(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Thief::update_dive, this);
	fsm::StateFunction update_dive();
	fsm::StateFunction update_land();
	fsm::StateFunction update_prepare();
	fsm::StateFunction update_dash();
	fsm::StateFunction update_stop();
	fsm::StateFunction update_laugh();
	fsm::StateFunction update_escape();
	fsm::StateFunction update_hurt();

  private:
	void teleport();

  private:
	automa::ServiceProvider* m_services;
	world::Map* m_map;

	std::optional<Drawable> m_loot;

	util::BitFlags<ThiefFlags> m_flags{};
	util::Cooldown m_respawn;

	bool change_state(ThiefState next, anim::Parameters params);
};

} // namespace fornani::enemy
