
#pragma once

#include "editor/canvas/Grid.hpp"

namespace pi {

class Layer {
  public:
	Layer(uint8_t const o, bool const c, sf::Vector2<uint32_t> dim) : grid{dim}, render_order(o), collidable(c), dimensions(dim) {}
	void clear();
	void erase();
	void set_position(sf::Vector2<float> to_position, float scale);
	Grid grid;
	uint8_t render_order{};
	bool collidable{};
	bool active{};
	sf::Vector2<uint32_t> dimensions{};
	std::string label{};
	char const* label_cstr{};
};

} // namespace pi
