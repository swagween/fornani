#include "Effect.hpp"
#include "../service/ServiceProvider.hpp"

namespace entity {

Effect::Effect(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Vector2<float> vel, int type, int index) : type(type) {
	if (index <= svc.assets.effect_lookup.size()) { sprite.setTexture(svc.assets.effect_lookup.at(index)); }
	auto framerate{16};
	switch (index) {
	case 0:
		//smoke
		sprite_dimensions = {36, 36};
		spritesheet_dimensions = {4, 7};
		break;
	case 1:
		sprite_dimensions = {54, 54};
		spritesheet_dimensions = {4, 8};
		break;
	case 2:
		sprite_dimensions = {12, 32};
		spritesheet_dimensions = {4, 4};
		sprite.setOrigin({6, 16});
		framerate = 8;
		break;
	case 3:
		sprite_dimensions = {192, 192};
		spritesheet_dimensions = {4, 10};
		break;
	case 4:
		sprite.setOrigin({16.f, 16.f});
		sprite_dimensions = {32, 32};
		spritesheet_dimensions = {1, 4};
		break;
	case 5:
		sprite.setOrigin({32.f, 32.f});
		sprite_dimensions = {64, 64};
		spritesheet_dimensions = {1, 5};
		break;
	case 6:
		sprite.setOrigin({16.f, 16.f});
		sprite_dimensions = {32, 32};
		spritesheet_dimensions = {1, 4};
		break;
	case 7:
		sprite.setOrigin({16.f, 16.f});
		sprite_dimensions = {32, 32};
		spritesheet_dimensions = {4, 4};
		break;
	case 8:
		//bullet hit
		sprite.setOrigin({16.f, 16.f});
		sprite_dimensions = {32, 32};
		spritesheet_dimensions = {4, 4};
		break;
	case 9:
		// doublejump
		sprite.setOrigin({24.f, 24.f});
		sprite_dimensions = {48, 48};
		spritesheet_dimensions = {1, 6};
		break;
	case 10:
		// dust
		sprite.setOrigin({32.f, 32.f});
		sprite_dimensions = {64, 64};
		spritesheet_dimensions = {1, 6};
		break;
	}
	animation.set_params({0, spritesheet_dimensions.y, framerate, 0});
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineColor(svc.styles.colors.fucshia);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize(static_cast<sf::Vector2<float>>(sprite_dimensions));
	physics = components::PhysicsComponent({0.99f, 0.99f}, 1.f);
	physics.position = pos;
	physics.velocity = vel;
}

void Effect::update(automa::ServiceProvider& svc, world::Map& map) {
	physics.update(svc);
	animation.update();
}

void Effect::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
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

void Effect::rotate() { sprite.rotate(90); }

} // namespace vfx