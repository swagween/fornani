
#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/Shape.hpp"

namespace components {
class CircleSensor {
  public:
	CircleSensor();
	CircleSensor(float radius);
	sf::CircleShape bounds{};
	bool within_bounds(shape::Shape& rect) const;
};

} // namespace components
