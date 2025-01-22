
#pragma once

#include <stdio.h>
#include "editor/canvas/Grid.hpp"
#include <fstream>

namespace pi {

class Layer {
  public:
	Layer() = default;
	Layer(uint8_t o, bool c, sf::Vector2<uint32_t> dim) : render_order(o), collidable(c), dimensions(dim), grid{dim} {}
	void clear();
	void erase();
	void set_position(sf::Vector2<float> to_position, float scale);
	Grid grid;
	uint8_t render_order{};
	bool collidable{};
	bool active{};
	sf::Vector2<uint32_t> dimensions{};
};

} // namespace pi
