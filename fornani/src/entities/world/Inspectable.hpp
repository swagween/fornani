
#pragma once

#include <string>
#include "../../utils/Shape.hpp"

namespace entity {

const uint32_t UNIT_SIZE = 32;

class Inspectable {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

	Inspectable() = default;
	Inspectable(Vecu16 dim, Vecu16 pos) : scaled_dimensions(dim), scaled_position(pos) {
		dimensions = static_cast<Vec>(dim * UNIT_SIZE);
		position = static_cast<Vec>(pos * UNIT_SIZE);
		bounding_box = shape::Shape(dimensions);
		bounding_box.set_position(position);
	}
	void update();
	void render(sf::RenderWindow& win, Vec campos); // for debugging

	Vec dimensions{};
	Vec position{};
	Vecu16 scaled_dimensions{};
	Vecu16 scaled_position{};
	shape::Shape bounding_box{};

	bool activated{};
	bool activate_on_contact{};

	std::string message{};
};

} // namespace entity
