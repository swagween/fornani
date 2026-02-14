
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/graphics/SpriteRotator.hpp>
#include <fornani/utils/Flaggable.hpp>

#define DUMPSTER_DIVER_BIND(f) std::bind(&DumpsterDiver::f, this)

namespace fornani::enemy {

enum class DumpsterDiverState { idle };
enum class DumpsterDiverVariant { breacher };
enum class DumpsterDiverFlags { emerging };

struct SurfacePoint {
	sf::Vector2f point{};
	int side{};
};

class DumpsterDiver final : public Enemy, public StateMachine<DumpsterDiverState>, public Flaggable<DumpsterDiverFlags> {
  public:
	DumpsterDiver(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	fsm::StateFunction state_function = std::bind(&DumpsterDiver::update_idle, this);
	fsm::StateFunction update_idle();

  private:
	DumpsterDiverVariant m_variant{};

	entity::Attack m_attack;
	components::SteeringBehavior m_steering{};
	util::Cooldown m_dive_time;
	util::Cooldown m_wait_time;
	std::vector<sf::Vector2f> m_home_points{};
	std::vector<SurfacePoint> m_surface_tiles{};

	bool change_state(DumpsterDiverState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
