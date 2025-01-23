
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

namespace pi {

class Camera {
  public:
	Camera(sf::Vector2<float> dimensions);

	void update() {
		bounding_box.position.x = position.x;
		bounding_box.position.y = position.y;
	}

	void move(sf::Vector2<float> distance) { position += distance; }
	void set_position(sf::Vector2<float> new_pos) { position = new_pos; }

	sf::Vector2<float> position{};
	sf::Rect<float> bounding_box{};
};

} // namespace pi
