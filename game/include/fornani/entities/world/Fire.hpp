#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/utils/Shape.hpp"
#include "fornani/particle/Sparkler.hpp"
#include "Inspectable.hpp"

namespace fornani::world {
class Map;
class Fire {
  public:
	Fire(automa::ServiceProvider& svc, sf::Vector2<float> position, int lookup);
	void update(automa::ServiceProvider& svc, player::Player& player, Map& map, gui::Console& console, dj::Json& set);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	private:
	int size{};
	shape::Shape bounding_box{};
	anim::AnimatedSprite sprite;
	vfx::Sparkler sparkler{};
	entity::Inspectable inspectable;
	sf::Vector2<float> sprite_offset{};
};

}