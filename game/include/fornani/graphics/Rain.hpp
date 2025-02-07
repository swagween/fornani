#pragma once

#include <SFML/Graphics.hpp>
#include "Droplet.hpp"
#include <vector>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::vfx {
class Rain {
  public:
	Rain(int intensity = 1, float fall_speed = 0.999f, float slant = -1.f, bool snow = false, bool collision = true);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
  private:
	std::vector<Droplet> drops{};
	int intensity{};
	float fall_speed{};
	float slant{};
	float variance{};
	float sway{};
	float z{};
	bool collision{};
	sf::Vector2<float> dimensions{};
	sf::RectangleShape raindrop{};
};
} // namespace fornani::vfx
