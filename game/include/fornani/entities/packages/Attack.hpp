#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/components/CircleSensor.hpp"

namespace fornani::player {
class Player;
}

namespace fornani::entity {

class Attack {
  public:
	Attack() = default;
	void update();
	void set_position(sf::Vector2f position);
	void handle_player(player::Player& player);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void enable();
	void disable();

	components::CircleSensor sensor{};
	components::CircleSensor hit{};
	sf::Vector2f origin{};
	sf::Vector2f hit_offset{};
};

} // namespace fornani::entity
