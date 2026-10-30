
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/utils/Cooldown.hpp>

namespace fornani::enemy {

enum class GeyserVariant : std::uint8_t { worm, toxic };

struct ToxicBubble {
	util::Cooldown lifetime;
	entity::Attack attack{};
	components::PhysicsComponent physics{};
};

class Geyser final : public Enemy {
  public:
	Geyser(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

  private:
	GeyserVariant m_variant{};
	automa::ServiceProvider* m_services;
	std::vector<ToxicBubble> m_bubbles{};
};

} // namespace fornani::enemy
