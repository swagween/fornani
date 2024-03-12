
#include "Drop.hpp"
#include "../../service/ServiceProvider.hpp"

namespace item {

Drop::Drop(automa::ServiceProvider& svc) {


	sprite.setTexture(svc.assets.t_heart);
	collider.bounding_box.dimensions = drop_dimensions;

	animation.params
}

void Drop::update() {

	animation.update();
}

void Drop::render(sf::RenderWindow& win, Vec campos) {

	sprite.setPosition((int)(collider.physics.position.x - campos.x), (int)(collider.physics.position.y - campos.y));
	win.draw(sprite);
	
}

int Drop::get_frame() { return anim.get_frame(); }
} // namespace item
