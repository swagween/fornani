#pragma once

#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/utils/Collider.hpp"
#include "fornani/particle/Gravitator.hpp"
#include "fornani/components/CircleSensor.hpp"
#include "fornani/entities/packages/Health.hpp"
#include "fornani/utils/StateFunction.hpp"
#define SPAWNABLE_PLAT_BIND(f) std::bind(&SpawnablePlatform::f, this)

namespace fornani::automa {
struct ServiceProvider;
}
namespace fornani::player {
class Player;
}

namespace fornani::entity {
enum class SpawnablePlatformState : std::uint8_t { open, opening, fading, closing, dormant };
class SpawnablePlatform {
  public:
	SpawnablePlatform(automa::ServiceProvider& svc, sf::Vector2f position, int index = 0);
	void update(automa::ServiceProvider& svc, player::Player& player, sf::Vector2f target);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	[[nodiscard]] auto get_index() const -> int { return index; }
	[[nodiscard]] auto collidable() const -> bool { return state == SpawnablePlatformState::open || state == SpawnablePlatformState::opening || state == SpawnablePlatformState::fading; }

  private:

	fsm::StateFunction state_function = std::bind(&SpawnablePlatform::update_dormant, this);
	fsm::StateFunction update_open();
	fsm::StateFunction update_opening();
	fsm::StateFunction update_fading();
	fsm::StateFunction update_closing();
	fsm::StateFunction update_dormant();
	bool change_state(SpawnablePlatformState next, std::string_view tag);

	SpawnablePlatformState state{};
	shape::Collider collider{};
	int index{};
	vfx::Gravitator gravitator{};
	components::CircleSensor sensor{};
	anim::AnimatedSprite sprite;
	Health health{};
};
} // namespace fornani::entity