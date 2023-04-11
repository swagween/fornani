//
//  Portal.cpp
//  entity
//
//

#include "Portal.hpp"

namespace entity {
	void Portal::update() {
		position = static_cast<Vec>(scaled_position * CELL_SIZE);
		dimensions = static_cast<Vec>(scaled_dimensions * CELL_SIZE);
		bounding_box.update(position.x, position.y, dimensions.x, dimensions.y);
	}
	void Portal::render(sf::RenderWindow& win, Vec campos) {
		sf::RectangleShape box{};
		if (activated) {
			box.setFillColor(sf::Color{ 80, 180, 120, 100 });
		} else {
			box.setFillColor(sf::Color{ 180, 120, 80, 100 });
		}
		box.setOutlineColor(sf::Color::White);
		box.setOutlineThickness(-1);
		box.setPosition(Vec(bounding_box.shape_x, bounding_box.shape_y) - campos);
		box.setSize(dimensions);
		//win.draw(box);
	}
} // end entity

/* Portal_cpp */
