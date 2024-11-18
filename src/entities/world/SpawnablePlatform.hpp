#pragma once

#include <string>
#include "../animation/AnimatedSprite.hpp"
#include "../../particle/Sparkler.hpp"
#include "../../utils/Collider.hpp"
#include "../../particle/Gravitator.hpp"
#include "../../components/CircleSensor.hpp"
#include "../packages/Health.hpp"
#include "../Entity.hpp"

namespace automa {
struct ServiceProvider;
}
namespace player {
class Player;
}

namespace entity {
enum class SpawnablePlatformState { open, dormant };
class SpawnablePlatform {
  public:
	SpawnablePlatform(automa::ServiceProvider& svc, sf::Vector2<float> position, int index = 0);
	void update(automa::ServiceProvider& svc, player::Player& player, sf::Vector2<float> target);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto get_index() const -> int { return index; }

  private:
	SpawnablePlatformState state{};
	shape::Collider collider{};
	int index{};
	sf::Vector2<float> root{};
	vfx::Gravitator gravitator{};
	components::CircleSensor sensor{};
	anim::AnimatedSprite sprite{};
	Health health{};
};
} // namespace entity