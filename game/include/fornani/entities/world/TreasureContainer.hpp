
#pragma once

#include <fornani/components/CircleSensor.hpp>
#include <fornani/components/SteeringComponent.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/entities/packages/Health.hpp>
#include <fornani/graphics/Animatable.hpp>

namespace fornani::arms {
class Projectile;
}

namespace fornani::entity {

enum class TreasureContainerState : std::uint8_t { neutral, shine };

class TreasureContainer : public Animatable {
  public:
	TreasureContainer(automa::ServiceProvider& svc, item::Rarity rarity, sf::Vector2f position, int index = 0);
	void update(automa::ServiceProvider& svc, sf::Vector2f target);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void submit(Renderer& renderer);
	std::unique_ptr<TreasureContainer> clone() const { return std::make_unique<TreasureContainer>(*this); }
	[[nodiscard]] auto destroyed() const -> bool { return m_health.is_dead(); }
	[[nodiscard]] auto get_index() const -> int { return index; }

  private:
	int index{};
	sf::Vector2f root{};
	item::Rarity rarity{};
	float loot_multiplier{};
	components::SteeringComponent m_steering{};
	components::CircleSensor sensor{};
	Health m_health;
	anim::Parameters m_neutral;
	anim::Parameters m_shine;
	TreasureContainerState m_state{};
};

} // namespace fornani::entity
