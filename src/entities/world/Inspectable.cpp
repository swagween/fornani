
#include "Inspectable.hpp"
#include "../player/Player.hpp"
#include "../../gui/Console.hpp"

namespace entity {

void Inspectable::update(automa::ServiceProvider& svc, player::Player& player, gui::Console& console) {
	position = static_cast<Vec>(scaled_position * UNIT_SIZE);
	dimensions = static_cast<Vec>(scaled_dimensions * UNIT_SIZE);
	bounding_box.set_position(position);
	activated = false;
	if (player.controller.inspecting() && bounding_box.overlaps(player.collider.hurtbox)) {
		activated = true;
	}
	if (activated) {
		console.set_source(svc.text.inspectables);
		console.load_and_launch(key);
	}
}

void Inspectable::render(sf::RenderWindow& win, Vec campos) {
	sf::RectangleShape box{};
	if (activated) {
		box.setFillColor(sf::Color{80, 180, 120, 100});
	} else {
		box.setFillColor(sf::Color{180, 120, 80, 100});
	}
	box.setOutlineColor(sf::Color::White);
	box.setOutlineThickness(-1);
	box.setPosition(bounding_box.position - campos);
	box.setSize(dimensions);
	win.draw(box);
}
} // namespace entity
