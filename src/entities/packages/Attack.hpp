#pragma once
#include <SFML/Graphics.hpp>
#include "../../components/CircleSensor.hpp"

namespace player {
class Player;
}

namespace entity {

class Attack {
  public:
	Attack() = default;
	void update();
	void set_position(sf::Vector2<float> position);
	void handle_player(player::Player& player);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);

	components::CircleSensor sensor{};
	components::CircleSensor hit{};
	sf::Vector2<float> origin{};
	sf::Vector2<float> hit_offset{};
};

} // namespace entity