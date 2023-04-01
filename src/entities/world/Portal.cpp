//
//  Portal.cpp
//  entity
//
//

#include "Portal.hpp"

namespace entity {
	void Portal::render(sf::RenderWindow& win, Vec campos) {
		sf::RectangleShape box{};
		box.setFillColor(sf::Color::Green);
		box.setOutlineColor(sf::Color::White);
		box.setPosition(position - campos);
		win.draw(box);
	}
} // end entity

/* Portal_cpp */
