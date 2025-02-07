
#pragma once

#include <SFML/Graphics.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::vfx {

class Scenery {
  public:
	Scenery(automa::ServiceProvider& svc, sf::Vector2<float> position, int style, int layer, int variant, float parallax = 1.f);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);

  private:
	sf::Vector2<int> dimensions{320, 320};
	sf::Vector2<float> f_dimensions{320.f, 320.f};
	sf::Vector2<float> position{};
	sf::Sprite sprite;
	struct {
		int style{};   // to match room style
		int layer{};   // draw depth
		int variant{}; // which sprite to draw
		float parallax{};
	} properties{};
};

} // namespace vfx
