
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/gui/BossHealth.hpp>
#include <fornani/particle/Chain.hpp>
#include <fornani/particle/Sparkler.hpp>

#define MIAAG_BIND(f) std::bind(&Miaag::f, this)

namespace fornani::enemy {

enum class MiaagState { idle, hurt, closed, dying, blinking, dormant, chomp, turn, awaken, spellcast };
enum class MiaagFlags { battle_mode, second_phase, gone };

class Miaag : public Enemy, public StateMachine<MiaagState> {
  public:
	Miaag(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	[[nodiscard]] auto invincible() const -> bool { return !flags.state.test(StateFlags::vulnerable); }
	[[nodiscard]] auto battle_mode() const -> bool { return m_flags.test(MiaagFlags::battle_mode); }
	[[nodiscard]] auto second_phase() const -> bool { return m_flags.test(MiaagFlags::second_phase); }
	[[nodiscard]] auto half_health() const -> bool { return health.get_quantity() < health.get_capacity() * 0.5f; }

	fsm::StateFunction state_function = std::bind(&Miaag::update_dormant, this);
	fsm::StateFunction update_dormant();
	fsm::StateFunction update_awaken();
	fsm::StateFunction update_idle();
	fsm::StateFunction update_hurt();
	fsm::StateFunction update_closed();
	fsm::StateFunction update_chomp();
	fsm::StateFunction update_spellcast();
	fsm::StateFunction update_turn();

  private:
	util::BitFlags<MiaagFlags> m_flags{};
	bool change_state(MiaagState next, anim::Parameters params);
	gui::BossHealth m_health_bar;
	std::unique_ptr<vfx::Chain> m_spine{};
	std::vector<int> m_spine_lookups{};
	sf::Sprite m_spine_sprite;

	components::SteeringBehavior m_steering{};
	entity::WeaponPackage m_magic;
	sf::Vector2f m_player_target{};
	sf::Vector2f m_target_point{};

	struct {
		util::Cooldown fire;
		util::Cooldown charge;
		util::Cooldown limit;
		util::Cooldown post_magic;
		util::Cooldown interlude;
		util::Cooldown chomped;
		util::Cooldown post_death;
	} m_cooldowns;

	automa::ServiceProvider* m_services;
	world::Map* m_map;
};

} // namespace fornani::enemy
