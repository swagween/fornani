
#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include <iostream>

namespace fornani::anim {

AnimatedSprite::AnimatedSprite(sf::Texture& texture, sf::Vector2<int> dimensions) : sprite(texture), dimensions(dimensions) {}

void AnimatedSprite::update(sf::Vector2<float> pos, int u, int v, bool horiz) {
	position = pos;
	horiz ? sprite.setTextureRect(sf::IntRect{{animation.get_frame() * dimensions.x, v * dimensions.y}, dimensions})
		  : sprite.setTextureRect(sf::IntRect{{u * dimensions.x, animation.get_frame() * dimensions.y}, dimensions});
	animation.update();
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineColor(sf::Color::Blue);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize(static_cast<sf::Vector2<float>>(dimensions));
}

void AnimatedSprite::push_params(std::string_view label, Parameters in_params) { params.insert({label, in_params}); }

void AnimatedSprite::set_params(std::string_view label, bool force) {
	if (params.contains(label)) {
		animation.set_params(params.at(label), force);
	} else {
		std::cout << "Invalid animation parameterization request in AnimatedSprite.cpp with label <" + std::string{label} + ">\n";
	}
}

void AnimatedSprite::set_dimensions(sf::Vector2<int> dim) { dimensions = dim; }

void AnimatedSprite::set_position(sf::Vector2<float> pos) { position = pos; }

void AnimatedSprite::set_scale(sf::Vector2<float> scale) { sprite.setScale(scale); }

void AnimatedSprite::set_origin(sf::Vector2<float> origin) { sprite.setOrigin(origin); }

void AnimatedSprite::set_texture(sf::Texture& texture) { sprite.setTexture(texture); }

void AnimatedSprite::random_start(automa::ServiceProvider& svc) {
	if (animation.params.duration > 1) { animation.frame.set(svc.random.random_range(0, animation.params.duration - 1)); }
}

void AnimatedSprite::handle_rotation(sf::Vector2<float> direction, int num_angles, bool radial) { rotator.handle_rotation(sprite, direction, num_angles, radial); }

void AnimatedSprite::end() { animation.end(); }

void AnimatedSprite::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam, bool debug) {
	if (!svc.greyblock_mode()) {
		sprite.setPosition(position - cam);
		win.draw(sprite);
	} else {
		sprite.setPosition(position - cam);
		win.draw(sprite);
		drawbox.setPosition(position - cam);
		win.draw(drawbox);
	}
	if (debug) { std::cout << position.y << "\n"; }
}

} // namespace anim
