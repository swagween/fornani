#include "Explosion.hpp"
#include "../service/ServiceProvider.hpp"

namespace entity {

Explosion::Explosion(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Vector2<float> vel, int type, int size) : type(type) {
	if (size <= svc.assets.explosion_lookup.size()) { sprite.setTexture(svc.assets.explosion_lookup.at(size)); }
	switch (size) {
	case 0:
		sprite_dimensions = {36, 36};
		spritesheet_dimensions = {4, 7};
		break;
	case 1:
		sprite_dimensions = {54, 54};
		spritesheet_dimensions = {4, 8};
		break;
	}
	animation.set_params({0, spritesheet_dimensions.y, 16, 0});
	drawbox.setFillColor(svc.styles.colors.navy_blue);
	drawbox.setSize(static_cast<sf::Vector2<float>>(sprite_dimensions));
	physics = components::PhysicsComponent({0.99f, 0.99f}, 1.f);
	physics.position = pos;
	physics.velocity = vel;
}

void Explosion::update(automa::ServiceProvider& svc, world::Map& map) {
	physics.update(svc);
	animation.update();
}

void Explosion::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	int u = type * sprite_dimensions.x;
	int v = animation.get_frame() * sprite_dimensions.y;
	sprite.setTextureRect({{u, v}, sprite_dimensions});
	sprite.setPosition(physics.position - cam);
	if (svc.greyblock_mode()) {
		drawbox.setPosition(physics.position - cam);
		win.draw(drawbox);
	} else {
		win.draw(sprite);
	}
}

} // namespace vfx