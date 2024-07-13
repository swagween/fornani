
#include "AnimatedSprite.hpp"
#include "../../service/ServiceProvider.hpp"
#include <iostream>

namespace anim {

AnimatedSprite::AnimatedSprite(sf::Texture& texture, sf::Vector2<int> dimensions) : sprite(texture), dimensions(dimensions) {}

void AnimatedSprite::update(sf::Vector2<float> pos, int u, int v) {
	position = pos;
	sprite.setTextureRect(sf::IntRect{{u * dimensions.x, animation.get_frame() * dimensions.y}, dimensions});
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
		std::cout << "Invalid animation parameterization request in AnimatedSprite.cpp\n";
	}
}

void AnimatedSprite::set_dimensions(sf::Vector2<int> dim) { dimensions = dim; }

void AnimatedSprite::set_origin(sf::Vector2<float> origin) { sprite.setOrigin(origin); }

void AnimatedSprite::end() { animation.end(); }

void AnimatedSprite::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (!svc.greyblock_mode()) {
		sprite.setPosition(position - cam);
		win.draw(sprite);
	} else {
		drawbox.setPosition(position - cam);
		win.draw(drawbox);
	}
}

} // namespace anim
