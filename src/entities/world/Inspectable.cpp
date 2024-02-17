
#include "Inspectable.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace entity {
void Inspectable::update() {
	position = static_cast<Vec>(scaled_position * UNIT_SIZE);
	dimensions = static_cast<Vec>(scaled_dimensions * UNIT_SIZE);
	bounding_box.set_position(position);
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
	if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
		win.draw(box);
		svc::counterLocator.get().at(svc::draw_calls)++;
	}
}
} // namespace entity
