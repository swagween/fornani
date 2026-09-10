
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/world/Mine.hpp>
#include <fornani/particle/Chain.hpp>

#define GLUB_BIND(f) std::bind(&Glub::f, this)

namespace fornani::enemy {

struct MineChain {
	std::unique_ptr<entity::Mine> mine;
	std::unique_ptr<vfx::Chain> chain;
	sf::Vector2f offset{};
	float timer{};
};

enum class GlubState : std::uint8_t { idle, turn };
enum class GlubVariant : std::uint8_t { naked, spikey };

class Glub final : public Enemy, public StateMachine<GlubState> {
  public:
	Glub(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Glub::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();

  private:
	bool change_state(GlubState next, anim::Parameters params);

  private:
	GlubVariant m_variant{};
	std::vector<MineChain> m_mines{};
	util::Cooldown m_spawn;

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
