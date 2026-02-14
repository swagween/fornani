
#pragma once

#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/entities/packages/Attack.hpp>

#define GRAND_MASTIFF_BIND(f) std::bind(&GrandMastiff::f, this)

namespace fornani::enemy {

enum class GrandMastiffState { idle, run, turn, slash, bite, growl, wag, die, howl };

class GrandMastiff final : public Boss, public StateMachine<GrandMastiffState> {
  public:
	GrandMastiff(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&GrandMastiff::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_run();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_slash();
	fsm::StateFunction update_bite();
	fsm::StateFunction update_growl();
	fsm::StateFunction update_wag();
	fsm::StateFunction update_howl();
	fsm::StateFunction update_die();

  private:
	entity::Attack m_bite;
	bool change_state(GrandMastiffState next, anim::Parameters params);

	util::Cooldown m_post_slash;
	util::Cooldown m_post_bite;

	void debug();
};

} // namespace fornani::enemy
