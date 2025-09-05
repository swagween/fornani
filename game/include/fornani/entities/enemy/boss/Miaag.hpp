
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/gui/BossHealth.hpp>
#include <fornani/particle/Sparkler.hpp>

#define MIAAG_BIND(f) std::bind(&Miaag::f, this)

namespace fornani::enemy {

enum class MiaagState : std::uint8_t { idle, hurt, closed, dying, blinking, dormant, chomp, turn, awaken };
enum class MiaagFlags : std::uint8_t { battle_mode, second_phase };

class Miaag : public Enemy {
  public:
	Miaag(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	[[nodiscard]] auto invincible() const -> bool { return !flags.state.test(StateFlags::vulnerable); }
	[[nodiscard]] auto half_health() const -> bool { return health.get_hp() < health.get_max() * 0.5f; }

	fsm::StateFunction state_function = std::bind(&Miaag::update_dormant, this);
	fsm::StateFunction update_dormant();
	fsm::StateFunction update_awaken();
	fsm::StateFunction update_idle();
	fsm::StateFunction update_hurt();
	fsm::StateFunction update_closed();
	fsm::StateFunction update_chomp();
	fsm::StateFunction update_turn();

  private:
	struct {
		MiaagState actual{};
		MiaagState desired{};
	} m_state{};
	util::BitFlags<MiaagFlags> m_flags{};
	void request(MiaagState to) { m_state.desired = to; }
	bool change_state(MiaagState next, anim::Parameters params);
	gui::BossHealth m_health_bar;
	vfx::Sparkler m_breath{};

	components::SteeringBehavior m_steering{};

	std::unordered_map<std::string, anim::Parameters> m_params;
	anim::Parameters const& get_params(std::string const& key);
};

} // namespace fornani::enemy
