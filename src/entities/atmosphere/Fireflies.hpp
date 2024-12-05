#pragma once

#include <vector>
#include "Firefly.hpp"

namespace automa {
struct ServiceProvider;
}

namespace vfx {
class Fireflies {
  public:
	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);

  private:
	std::vector<Firefly> flies{};
};
} // namespace npc
