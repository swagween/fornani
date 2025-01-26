#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/components/CircleSensor.hpp"
#include "fornani/utils/Cooldown.hpp"

namespace world {
class Map;
}
namespace player {
class Player;
}
namespace automa {
struct ServiceProvider;
}

namespace entity {

class Shockwave {
  public:
	Shockwave() = default;
	Shockwave(sf::Vector2<float> speed);
	void start(int time = 600);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void set_position(sf::Vector2<float> to_position);
	void handle_player(player::Player& player);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);

	components::CircleSensor hit{};
	sf::Vector2<float> position{};
	sf::Vector2<float> origin{};
	sf::Vector2<float> speed{};
	util::Cooldown lifetime{};
};

} // namespace entity
