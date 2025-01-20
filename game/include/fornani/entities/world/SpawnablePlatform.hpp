#pragma once

#include <string>
#include "../animation/AnimatedSprite.hpp"
#include "../../particle/Sparkler.hpp"
#include "../../utils/Collider.hpp"
#include "../../particle/Gravitator.hpp"
#include "../../components/CircleSensor.hpp"
#include "../../../include/fornani/entities/packages/Health.hpp"
#include "../Entity.hpp"
#include "../../utils/StateFunction.hpp"
#define SPAWNABLE_PLAT_BIND(f) std::bind(&SpawnablePlatform::f, this)

namespace automa {
struct ServiceProvider;
}
namespace player {
class Player;
}

namespace entity {
enum class SpawnablePlatformState { open, opening, fading, closing, dormant };
class SpawnablePlatform {
  public:
	SpawnablePlatform(automa::ServiceProvider& svc, sf::Vector2<float> position, int index = 0);
	void update(automa::ServiceProvider& svc, player::Player& player, sf::Vector2<float> target);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto get_index() const -> int { return index; }
	[[nodiscard]] auto collidable() const -> bool { return state == SpawnablePlatformState::open || state == SpawnablePlatformState::fading; }

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
} // namespace entity