#pragma once

#include <string>
#include "fornani/entities/item/Drop.hpp"
#include "fornani/particle/Gravitator.hpp"
#include "fornani/components/CircleSensor.hpp"
#include "fornani/entities/packages/Health.hpp"

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace arms {
class Projectile;
}

namespace entity {
class TreasureContainer {
  public:
	TreasureContainer(automa::ServiceProvider& svc, item::Rarity rarity, sf::Vector2<float> position, int index = 0);
	void update(automa::ServiceProvider& svc, sf::Vector2<float> target);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto destroyed() const -> bool { return health.is_dead(); }
	[[nodiscard]] auto get_index() const -> int { return index; }
  private:
	int index{};
	sf::Vector2<float> root{};
	item::Rarity rarity{};
	float loot_multiplier{};
	vfx::Gravitator gravitator{};
	components::CircleSensor sensor{};
	sf::Sprite sprite;
	Health health{};
};

} // namespace entity
