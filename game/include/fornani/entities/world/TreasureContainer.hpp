#pragma once

#include <fornani/components/CircleSensor.hpp>
#include <fornani/entities/item/Drop.hpp>
#include <fornani/entities/packages/Health.hpp>
#include <fornani/particle/Gravitator.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::arms {
class Projectile;
}

namespace fornani::entity {

enum class TreasureContainerState : std::uint8_t { neutral, shine };

class TreasureContainer : public Animatable {
  public:
	TreasureContainer(automa::ServiceProvider& svc, item::Rarity rarity, sf::Vector2f position, int index = 0);
	void update(automa::ServiceProvider& svc, sf::Vector2f target);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	[[nodiscard]] auto destroyed() const -> bool { return health.is_dead(); }
	[[nodiscard]] auto get_index() const -> int { return index; }

  private:
	int index{};
	sf::Vector2f root{};
	item::Rarity rarity{};
	float loot_multiplier{};
	vfx::Gravitator gravitator{};
	components::CircleSensor sensor{};
	Health health{};
	anim::Parameters m_neutral;
	anim::Parameters m_shine;
	TreasureContainerState m_state{};
};

} // namespace fornani::entity
