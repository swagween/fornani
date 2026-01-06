
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/CircleSensor.hpp>
#include <fornani/utils/Cooldown.hpp>

namespace fornani::player {
class Player;
}

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {

class Map;

class Explosion {
  public:
	Explosion(automa::ServiceProvider& svc, sf::Vector2f position, float radius);
	void update(automa::ServiceProvider& svc, player::Player& player, Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);

	[[nodiscard]] auto is_done() const -> bool { return m_lifetime.is_almost_complete(); }

  private:
	components::CircleSensor m_sensor;
	util::Cooldown m_lifetime;
};

} // namespace fornani::world
