
#include "Drop.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace item {

Drop::Drop(Vec dim, Vec pos) {
	collider.bounding_box.dimensions = dim;
	collider.physics.position = pos;
}

void Drop::update() {

	anim.update();
	anim.current_state.update();
}

void Drop::render(sf::RenderWindow& win, Vec campos) {

	sprite.setPosition((int)(collider.physics.position.x - campos.x), (int)(collider.physics.position.y - campos.y));
	win.draw(sprite);
	
}

int Drop::get_frame() { return anim.get_frame(); }
} // namespace item
