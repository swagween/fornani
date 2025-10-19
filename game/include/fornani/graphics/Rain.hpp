#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Droplet.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::vfx {
class Rain {
  public:
	explicit Rain(int intensity = 1, float fall_speed = 0.999f, float slant = -1.f, bool snow = false, bool collision = true);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);

  private:
	std::vector<Droplet> drops{};
	int intensity{};
	float fall_speed{};
	float slant{};
	float variance{};
	float sway{};
	float z{};
	bool collision{};
	sf::Vector2f dimensions{};
	sf::RectangleShape raindrop{};
};
} // namespace fornani::vfx
