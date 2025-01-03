
#pragma once
#include <SFML/Graphics.hpp>
#include "../animation/AnimatedSprite.hpp"
#include "../../utils/Shape.hpp"
#include "../../particle/Sparkler.hpp"
#include "Inspectable.hpp"

namespace world {
class Map;
class Fire {
  public:
	Fire(automa::ServiceProvider& svc, sf::Vector2<float> position);
	void update(automa::ServiceProvider& svc, player::Player& player, Map& map, gui::Console& console, dj::Json& set);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	private:
	shape::Shape bounding_box{};
	anim::AnimatedSprite sprite;
	vfx::Sparkler sparkler{};
	entity::Inspectable inspectable;
	sf::Vector2<float> sprite_offset{};
};

}