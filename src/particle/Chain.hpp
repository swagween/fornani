
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Spring.hpp"

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace vfx {
class Chain {
  public:
	Chain(SpringParameters params, int num_links);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
  private:
	std::vector<Spring> links{};
};

} // namespace vfx
