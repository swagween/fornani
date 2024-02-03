//
//  SavePoint.cpp
//  entity
//
//

#include "SavePoint.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace entity {

	void SavePoint::update() {
	}

	void SavePoint::render(sf::RenderWindow& win, Vec campos) {
		sprite.setPosition((int)(position.x - campos.x), (int)(position.y - campos.y));
		win.draw(sprite);
		svc::counterLocator.get().at(svc::draw_calls)++;
		sf::RectangleShape box{};
		if (activated) {
			box.setFillColor(sf::Color{ 80, 180, 120, 100 });
		}
		else {
			box.setFillColor(sf::Color{ 180, 120, 80, 100 });
		}
		box.setOutlineColor(sf::Color::White);
		box.setOutlineThickness(-1);
		box.setPosition(bounding_box.position - campos);
		box.setSize(bounding_box.dimensions);
		win.draw(box);
		svc::counterLocator.get().at(svc::draw_calls)++;
	}

} // end entity

/* SavePoint_cpp */
