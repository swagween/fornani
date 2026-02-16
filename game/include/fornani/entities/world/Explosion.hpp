
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/CircleSensor.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani::player {
class Player;
}

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {

class Map;

enum class ExplosionFlags { exhausted };

struct ExplosionSpecifications {
	std::string tag{};
	std::string emitter{};
	arms::Team team{};
	sf::Vector2f position{};
	float radius{};
	int channel{};
	int volatility{};
	util::Cooldown delay{24};
};

class Explosion : public Flaggable<ExplosionFlags> {
  public:
	Explosion(automa::ServiceProvider& svc, arms::Team team, sf::Vector2f position, float radius);
	void update(automa::ServiceProvider& svc, player::Player& player, Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);

	[[nodiscard]] auto is_done() const -> bool { return m_lifetime.is_almost_complete(); }

  private:
	components::CircleSensor m_sensor;
	util::Cooldown m_lifetime;
	arms::Team m_team;
};

} // namespace fornani::world
