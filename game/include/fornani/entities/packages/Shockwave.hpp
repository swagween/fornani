
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/CircleSensor.hpp>
#include <fornani/utils/Cooldown.hpp>

namespace fornani::world {
class Map;
}

namespace fornani::player {
class Player;
}

namespace fornani::automa {
struct ServiceProvider;
}

struct ShockwaveParameters {
	int frequency{};
	int lifetime{};
	int style{};
	sf::Vector2f speed{};
};

namespace fornani::entity {

class Shockwave {
  public:
	Shockwave() = default;
	explicit Shockwave(ShockwaveParameters parameters);
	void start();
	void update(automa::ServiceProvider& svc, world::Map& map);
	void set_position(sf::Vector2f to_position);
	void handle_player(player::Player& player);
	void render(sf::RenderWindow& win, sf::Vector2f cam);

	components::CircleSensor hit{};
	sf::Vector2f position{};
	sf::Vector2f origin{};
	util::Cooldown lifetime{};

  private:
	ShockwaveParameters m_parameters{};
};

} // namespace fornani::entity
