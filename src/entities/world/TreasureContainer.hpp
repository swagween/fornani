#pragma once

#include <string>
#include "../item/Drop.hpp"
#include "../../particle/Gravitator.hpp"
#include "../../components/CircleSensor.hpp"
#include "../packages/Health.hpp"

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
	item::Rarity rarity{};
	vfx::Gravitator gravitator{};
	components::CircleSensor sensor{};
	sf::Sprite sprite{};
	Health health{};
};

} // namespace entity
