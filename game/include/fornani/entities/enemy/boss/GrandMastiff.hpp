
#pragma once

#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Shockwave.hpp>

#define GRAND_MASTIFF_BIND(f) std::bind(&GrandMastiff::f, this)

namespace fornani::enemy {

enum class GrandMastiffState : std::uint8_t { idle, run, turn, slash, bite, growl, wag, die, howl, pound, begin_howl };

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
	fsm::StateFunction update_begin_howl();
	fsm::StateFunction update_howl();
	fsm::StateFunction update_pound();
	fsm::StateFunction update_die();

  private:
	bool change_state(GrandMastiffState next, anim::Parameters params);
	void spawn_bite();
	void debug();

  private:
	entity::Attack m_bite;
	std::array<entity::Attack, 3> m_attacks{};
	std::vector<entity::Shockwave> m_shockwaves{};
	std::optional<Animatable> m_bite_effect{};
	GrandMastiffState m_last_move{};
	std::vector<GrandMastiffState> m_moveset{};
	std::unordered_map<GrandMastiffState, std::size_t> m_move_counts;

	util::Cooldown m_post_slash;
	util::Cooldown m_post_bite;
	util::Cooldown m_post_howl;
	util::Counter m_howl_count;
	util::Cooldown m_attack_timer{500};

	entity::WeaponPackage m_demon_star;

	sf::Vector2f m_bite_target{};
	sf::Vector2f m_player_position{};
};

} // namespace fornani::enemy
