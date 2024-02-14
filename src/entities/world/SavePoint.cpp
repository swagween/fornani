//
//  SavePoint.cpp
//  entity
//
//

#include "SavePoint.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace entity {

	SavePoint::SavePoint() {
		id = -1;
		dimensions = { 32, 32 };
		bounding_box = shape::Shape(dimensions);
	}

	void SavePoint::update() {

		position = static_cast<Vec>(scaled_position) * 32.f;
		bounding_box.set_position(position);
		activated = false;

		if(svc::playerLocator.get().flags.input.test(Input::inspecting)) {
			if (svc::playerLocator.get().collider.bounding_box.SAT(bounding_box)) {

				if (can_activate) {
					activated = true;
					save();
				}

			}
		} else {
			can_activate = true;
		}

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

	void SavePoint::save() {

		svc::dataLocator.get().save_progress(id);
		can_activate = false;

	}

} // end entity

/* SavePoint_cpp */
