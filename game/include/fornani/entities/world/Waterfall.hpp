
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/physics/Shape.hpp>
#include <fornani/utils/IWorldPositionable.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::world {

class Map;

class Waterfall : public Animatable, public IWorldPositionable {
  public:
	Waterfall(automa::ServiceProvider& svc, Map& map, sf::Vector2u position);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);

  private:
	shape::Shape m_bounding_box{};
	shape::Shape m_detector{};
};

} // namespace fornani::world
