
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string_view>
#include "../utils/Collider.hpp"
#include "../utils/Cooldown.hpp"

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace vfx {

class Particle {
  public:
	Particle(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Vector2<float> dim, std::string_view type, sf::Color color, dir::Direction direction);
	void update(world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto done() const -> bool { return lifespan.is_complete(); }

  private:
	sf::RectangleShape box{};
	sf::Vector2<float> position{};
	sf::Vector2<float> dimensions{};
	util::Cooldown lifespan{};
	shape::Collider collider{};
	int frame{};
};

} // namespace vfx
