#pragma once

#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/entities/world/Inspectable.hpp"
#include "fornani/particle/Sparkler.hpp"
#include "fornani/utils/Shape.hpp"

#include <SFML/Graphics.hpp>

#include <optional>

namespace fornani::world {
class Map;
class Fire {
  public:
	Fire(automa::ServiceProvider& svc, sf::Vector2<float> position, int lookup);
	void update(automa::ServiceProvider& svc, player::Player& player, Map& map, std::optional<std::unique_ptr<gui::Console>>& console, dj::Json& set);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);

  private:
	int size{};
	shape::Shape bounding_box{};
	anim::AnimatedSprite sprite;
	vfx::Sparkler sparkler{};
	entity::Inspectable inspectable;
	sf::Vector2<float> sprite_offset{};
};

} // namespace fornani::world
